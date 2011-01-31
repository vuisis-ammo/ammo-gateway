/* soapContentServiceSoapBindingProxy.h
   Generated by gSOAP 2.7.17 from ContentService.h
   Copyright(C) 2000-2010, Robert van Engelen, Genivia Inc. All Rights Reserved.
   This part of the software is released under one of the following licenses:
   GPL, the gSOAP public license, or Genivia's license for commercial use.
*/

#ifndef soapContentServiceSoapBindingProxy_H
#define soapContentServiceSoapBindingProxy_H
#include "soapH.h"

class SOAP_CMAC ContentServiceSoapBindingProxy : public soap
{ public:
	/// Endpoint URL of service 'ContentServiceSoapBindingProxy' (change as needed)
	const char *soap_endpoint;
	/// Constructor
	ContentServiceSoapBindingProxy();
	/// Constructor with copy of another engine state
	ContentServiceSoapBindingProxy(const struct soap&);
	/// Constructor with engine input+output mode control
	ContentServiceSoapBindingProxy(soap_mode iomode);
	/// Constructor with engine input and output mode control
	ContentServiceSoapBindingProxy(soap_mode imode, soap_mode omode);
	/// Destructor frees deserialized data
	virtual	~ContentServiceSoapBindingProxy();
	/// Initializer used by constructors
	virtual	void ContentServiceSoapBindingProxy_init(soap_mode imode, soap_mode omode);
	/// Delete all deserialized data (uses soap_destroy and soap_end)
	virtual	void destroy();
	/// Disables and removes SOAP Header from message
	virtual	void soap_noheader();
	/// Put SOAP Header in message
	virtual	void soap_header(struct _wsse__Security *wsse__Security, char *wsa5__MessageID, struct wsa5__RelatesToType *wsa5__RelatesTo, struct wsa5__EndpointReferenceType *wsa5__From, struct wsa5__EndpointReferenceType *wsa5__ReplyTo, struct wsa5__EndpointReferenceType *wsa5__FaultTo, char *wsa5__To, char *wsa5__Action);
	/// Get SOAP Header structure (NULL when absent)
	virtual	const SOAP_ENV__Header *soap_header();
	/// Get SOAP Fault structure (NULL when absent)
	virtual	const SOAP_ENV__Fault *soap_fault();
	/// Get SOAP Fault string (NULL when absent)
	virtual	const char *soap_fault_string();
	/// Get SOAP Fault detail as string (NULL when absent)
	virtual	const char *soap_fault_detail();
	/// Force close connection (normally automatic, except for send_X ops)
	virtual	int soap_close_socket();
	/// Print fault
	virtual	void soap_print_fault(FILE*);
#ifndef WITH_LEAN
	/// Print fault to stream
	virtual	void soap_stream_fault(std::ostream&);
	/// Put fault into buffer
	virtual	char *soap_sprint_fault(char *buf, size_t len);
#endif

	/// Web service operation 'CreateOperation' (returns error code or SOAP_OK)
	virtual	int CreateOperation(ns2__CreateType *ns2__Create, ns2__CreateResponseType *ns2__CreateResponse);

	/// Web service operation 'GetOperation' (returns error code or SOAP_OK)
	virtual	int GetOperation(ns2__GetType *ns2__Get, ns2__GetResponseType *ns2__GetResponse);

	/// Web service operation 'DeleteOperation' (returns error code or SOAP_OK)
	virtual	int DeleteOperation(ns2__DeleteType *ns2__Delete, struct __ns4__DeleteOperationResponse &_param_1);

	/// Web service operation 'UpdateOperation' (returns error code or SOAP_OK)
	virtual	int UpdateOperation(ns2__UpdateType *ns2__Update, struct __ns4__UpdateOperationResponse &_param_2);

	/// Web service operation 'GetCapabilitiesOperation' (returns error code or SOAP_OK)
	virtual	int GetCapabilitiesOperation(ns3__GetCapabilitiesType *ns3__GetCapabilities, ns2__ContentServiceCapabilitiesType *ns2__ContentServiceCapabilities);
};
#endif
