/*** Copyright (c), The Regents of the University of California            ***
 *** For more information please refer to files in the COPYRIGHT directory ***/
/* This is script-generated code (for the most part).  */
/* See ooiGenServReq.h for a description of this API call.*/
#include "ooiGenServReq.h"
#include "rodsLog.h"
#include "rsGlobalExtern.h"
#include "rcGlobalExtern.h"
#include "rsApiHandler.h"
#include "specColl.h"
#include "getRemoteZoneResc.h"

/* XXXX these will be defined in a OOI resource */
#define OOI_GATEWAY_URL "http://localhost"
#define OOI_GATEWAY_PORT                "5000"

int
rsOoiGenServReq (rsComm_t *rsComm, ooiGenServReqInp_t *ooiGenServReqInp,
ooiGenServReqOut_t **ooiGenServReqOut)
{
    int status;

    status = _rsOoiGenServReq (rsComm, ooiGenServReqInp, ooiGenServReqOut);

    return status;
}

int
_rsOoiGenServReq (rsComm_t *rsComm, ooiGenServReqInp_t *ooiGenServReqInp,
ooiGenServReqOut_t **ooiGenServReqOut)
{
    CURL *easyhandle;
    CURLcode res;
    char myUrl[MAX_NAME_LEN];
    int status;
    char *postStr = NULL;
    ooiGenServReqStruct_t ooiGenServReqStruct;

    easyhandle = curl_easy_init();
    if(!easyhandle) {
        rodsLog (LOG_ERROR, 
          "_rsOoiGenServReq: curl_easy_init error");
        return OOI_CURL_EASY_INIT_ERR;
    }
    snprintf (myUrl, MAX_NAME_LEN, "%s:%s/%s/%s/%s",
      OOI_GATEWAY_URL, OOI_GATEWAY_PORT, ION_SERVICE_STR,
        ooiGenServReqInp->servName, ooiGenServReqInp->servOpr);

    if (ooiGenServReqInp->params.len > 0) {
        /* do POST */
        status = jsonPackOoiServReqForPost (ooiGenServReqInp->servName,
                                        ooiGenServReqInp->servOpr,
                                        &ooiGenServReqInp->params, &postStr);
        if (status < 0) {
            rodsLogError (LOG_ERROR, status,
              "_rsOoiGenServReq: jsonPackOoiServReq error");
            return status;
        }
        curl_easy_setopt(easyhandle, CURLOPT_POSTFIELDS, postStr);
    }
    curl_easy_setopt(easyhandle, CURLOPT_URL, myUrl);
    curl_easy_setopt(easyhandle, CURLOPT_WRITEFUNCTION, ooiGenServReqFunc);
    bzero (&ooiGenServReqStruct, sizeof (ooiGenServReqStruct));
    ooiGenServReqStruct.outType = ooiGenServReqInp->outType;
    ooiGenServReqStruct.outInx = ooiGenServReqInp->outInx;

    curl_easy_setopt(easyhandle, CURLOPT_WRITEDATA, &ooiGenServReqStruct);

    res = curl_easy_perform (easyhandle);
    free (postStr);

    if (res != CURLE_OK) {
	/* res is +ive for error */
        rodsLog (LOG_ERROR, 
          "_rsOoiGenServReq: curl_easy_perform error: %d", res);
	free (*ooiGenServReqOut);
        *ooiGenServReqOut = NULL;
        return OOI_CURL_EASY_PERFORM_ERR - res;
    }
    *ooiGenServReqOut = ooiGenServReqStruct.ooiGenServReqOut;
    curl_easy_cleanup (easyhandle);

    return 0;
}

size_t
ooiGenServReqFunc (void *buffer, size_t size, size_t nmemb, void *userp)
{
    char *type_PI;
    int status;
    void *ptr = NULL;
    json_t *root, *dataObj, *responseObj;
    json_error_t jerror;

    ooiGenServReqStruct_t *ooiGenServReqStruct = 
      (ooiGenServReqStruct_t *) userp;

    root = json_loads((const char*) buffer, 0, &jerror);
    if (!root) {
        rodsLog (LOG_ERROR,
          "jsonUnpackOoiRespStr: json_loads error. %s", jerror.text);
        return OOI_JSON_LOAD_ERR;
    }
    dataObj = json_object_get (root, OOI_DATA_TAG);
    if (!dataObj) {
       rodsLog (LOG_ERROR,
          "jsonUnpackOoiRespStr: json_object_get data failed.");
        json_decref (root);
        return OOI_JSON_GET_ERR;
    }
    responseObj = json_object_get(dataObj, OOI_GATEWAY_RESPONSE_TAG);
    if (!responseObj) {
        responseObj = json_object_get(dataObj, OOI_GATEWAY_ERROR_TAG);
        if (!responseObj) {
            json_decref (root);
            rodsLog (LOG_ERROR,
              "jsonUnpackOoiRespStr: json_object_get GatewayResponse failed.");
            return OOI_JSON_GET_ERR;
	} else {
            rodsLog (LOG_ERROR,
              "jsonUnpackOoiRespStr: Gateway returns %s", (char *) buffer);
            ooiGenServReqStruct->ooiGenServReqOut =
              (ooiGenServReqOut_t *) calloc (1, sizeof (ooiGenServReqOut_t));

            rstrcpy (ooiGenServReqStruct->ooiGenServReqOut->type_PI, 
              STR_MS_T, NAME_LEN);
            ooiGenServReqStruct->ooiGenServReqOut->ptr = 
              strdup ((char *)buffer);
            return OOI_JSON_GET_ERR;
        }
    }

    switch (ooiGenServReqStruct->outType) {
      case OOI_STR_TYPE:
	type_PI = STR_MS_T;
        status = jsonUnpackOoiRespStr (responseObj, (char **) &ptr);
        break;
      case OOI_DICT_TYPE:
	type_PI = Dictionary_MS_T;
        status = jsonUnpackOoiRespDict (responseObj, (dictionary_t **) &ptr);
        break;
      case OOI_DICT_ARRAY_TYPE:
	type_PI = DictArray_MS_T;
        status = jsonUnpackOoiRespDictArray (responseObj, 
             (dictArray_t **) &ptr);
        break;
      case OOI_DICT_ARRAY_IN_ARRAY:
	type_PI = DictArray_MS_T;
       status = jsonUnpackOoiRespDictArrInArr (responseObj, 
           (dictArray_t **) &ptr, ooiGenServReqStruct->outInx);
        break;
      case OOI_LIST_TYPE:
        type_PI = Dictionary_MS_T;
        status = jsonUnpackOoiRespList (responseObj, (dictionary_t **) &ptr);
        break;
      default:
        rodsLog (LOG_ERROR,
          "ooiGenServReqFunc: outType %d not supported", 
          ooiGenServReqStruct->outType);
        status = OOI_JSON_TYPE_ERR;
    }
    json_decref (root);
    if (status < 0) return 0;

    ooiGenServReqStruct->ooiGenServReqOut =
      (ooiGenServReqOut_t *) calloc (1, sizeof (ooiGenServReqOut_t));

    rstrcpy (ooiGenServReqStruct->ooiGenServReqOut->type_PI, type_PI, NAME_LEN);
    ooiGenServReqStruct->ooiGenServReqOut->ptr = ptr;

    return nmemb*size;
}

