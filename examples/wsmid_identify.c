/*******************************************************************************
 * Copyright (C) 2004-2006 Intel Corp. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  - Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 *  - Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 *  - Neither the name of Intel Corp. nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL Intel Corp. OR THE CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *******************************************************************************/

/**
 * @author Anas Nashif
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

#include "u/libu.h"

#include "wsman-client-api.h"
#include "wsman-client-transport.h"
#include "wsman-xml-serializer.h"

struct __wsmid_identify
{
	XML_TYPE_STR ProtocolVersion;
	XML_TYPE_STR ProductVendor;
	XML_TYPE_STR ProductVersion;
};
typedef struct __wsmid_identify wsmid_identify;

SER_START_ITEMS(wsmid_identify)
SER_STR("ProtocolVersion", 1), 
SER_STR("ProductVendor", 1),
SER_STR("ProductVersion", 1),
SER_END_ITEMS(wsmid_identify);


static char  vendor = 0;
static char version = 0;
static char protocol = 0;
static char *endpoint = NULL;

int main(int argc, char** argv)
{
	
    WsManClient *cl;
    WsXmlDocH doc;
    actionOptions options;
    char retval = 0;
    u_error_t *error = NULL;

    u_option_entry_t opt[] = {
    { "product",	'p',	U_OPTION_ARG_NONE,	&vendor,
		"Print Product Vendor",	NULL },
    { "version",	'v',	U_OPTION_ARG_NONE,	&version,
		"Print Product Version",	NULL  },
    { "protocol",	'P',	U_OPTION_ARG_NONE,	&protocol,
		"Print Protocol Version",	NULL  },
    { "endpoint",	'u',	U_OPTION_ARG_STRING,	&endpoint,
		"Endpoint in form of a URL", "<uri>" },
    { NULL }
    };

    u_option_context_t *opt_ctx;	
    opt_ctx = u_option_context_new("");
    u_option_context_set_ignore_unknown_options(opt_ctx, FALSE);
    u_option_context_add_main_entries(opt_ctx, opt, "wsmid_identify");
    retval = u_option_context_parse(opt_ctx, &argc, &argv, &error);

    u_option_context_free(opt_ctx);

    if (error) {
      if (error->message)
        printf ("%s\n", error->message);
      u_error_free(error);
      return 1;
    }
    u_error_free(error);


    u_uri_t *uri;
    if (endpoint) {
      u_uri_parse((const char *)endpoint, &uri);
    }
    if (!endpoint || !uri) {
      fprintf(stderr, "endpoint option required\n");
      return 1;
    }


    wsman_client_transport_init(NULL);
    cl = wsman_create_client( uri->host,
        uri->port,
        uri->path,
        uri->scheme,
        uri->user,
        uri->pwd);		
    initialize_action_options(&options);


    doc = wsman_identify(cl, options);

    WsXmlNodeH soapBody = ws_xml_get_soap_body(doc);
    if (ws_xml_get_child(soapBody, 0, XML_NS_WSMAN_ID, "IdentifyResponse")) {
         wsmid_identify *id = ws_deserialize(wsman_client_get_context(cl),
                                     soapBody,
                                     wsmid_identify_TypeInfo, "IdentifyResponse",
                                     XML_NS_WSMAN_ID, XML_NS_WSMAN_ID,
                                     0, 0);

         if (vendor)
           printf("%s\n", id->ProductVendor);
         if (version)
           printf("%s\n", id->ProductVersion);
         if (protocol)
           printf("%s\n", id->ProtocolVersion);

         if (!protocol && !vendor && !version ) {
             printf("\n");
           printf("%s %s supporting protocol %s\n", id->ProductVendor, id->ProductVersion,id->ProtocolVersion);
         }
           
    }
    if (uri) {
      u_uri_free(uri);
    }
    
    if (doc) {			
        ws_xml_destroy_doc(doc);
    }

    destroy_action_options(&options);
    wsman_release_client(cl);

	
	return 0;
}


