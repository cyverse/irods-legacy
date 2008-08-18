#include "integritychecksMS.h"
#include "icutils.h"


/* Utility function for listing file object and an input parameter field */
int msiListFields (msParam_t *mPin1, msParam_t *mPin2, msParam_t *mPout1, msParam_t* mPout2, ruleExecInfo_t *rei) {

	genQueryInp_t gqin;
	genQueryOut_t *gqout = NULL;
	char condStr[MAX_NAME_LEN];
	char tmpstr[MAX_NAME_LEN];
	rsComm_t *rsComm;
	char* collname;
	char* fieldname;
	sqlResult_t *dataName;
	sqlResult_t *dataField;
	bytesBuf_t*	mybuf=NULL;
	int i,j;
	int	fieldid;

	RE_TEST_MACRO ("    Calling msiListFields")

	/* Sanity check */
	if (rei == NULL || rei->rsComm == NULL) {
		rodsLog (LOG_ERROR, "msiListFields: input rei or rsComm is NULL");
		return (SYS_INTERNAL_NULL_INPUT_ERR);
	}

	rsComm = rei->rsComm;

	/* init stuff */
	memset (&gqin, 0, sizeof(genQueryInp_t));
	gqin.maxRows = MAX_SQL_ROWS;
    mybuf = (bytesBuf_t *)malloc(sizeof(bytesBuf_t));
    memset (mybuf, 0, sizeof (bytesBuf_t));

	/* construct an SQL query from the parameter list */
	collname = (char*) strdup (mPin1->inOutStruct);
	fieldname = (char*) strdup (mPin2->inOutStruct);
	fieldid = getAttrIdFromAttrName(fieldname);

	rodsLog (LOG_ERROR, "fieldname: %s\tfieldid:%d", fieldname,fieldid);

	/* this is the info we want returned from the query */
	addInxIval (&gqin.selectInp, COL_DATA_NAME, 1);
	addInxIval (&gqin.selectInp, fieldid, 1);
	snprintf (condStr, MAX_NAME_LEN, " = '%s'", collname);
	addInxVal (&gqin.sqlCondInp, COL_COLL_NAME, condStr);

	rodsLog (LOG_ERROR, "got here 1");

	j = rsGenQuery (rsComm, &gqin, &gqout);

	if (j != CAT_NO_ROWS_FOUND) {

		rodsLog (LOG_ERROR, "got here 2");

		printGenQueryOut(stderr, NULL, NULL, gqout);

		dataName = getSqlResultByInx (gqout, COL_DATA_NAME);
		dataField = getSqlResultByInx (gqout, fieldid);

		rodsLog (LOG_ERROR, "got here 3 rowCnt=%d",gqout->rowCnt);

		for (i=0; i<gqout->rowCnt; i++) {
			sprintf (tmpstr, "Data object:%s\t%s:%s\n", &dataName->value[dataName->len *i], fieldname, &dataField->value[dataField->len *i]);
		rodsLog (LOG_ERROR, "got here 4");
			appendToByteBuf (mybuf, tmpstr);
		}

	} 

	fillBufLenInMsParam (mPout1, mybuf->len, mybuf);
	fillIntInMsParam (mPout2, rei->status);
  
	return(rei->status);

}


/* Silly hello world microservice */
int msiHiThere (ruleExecInfo_t *rei) {

	int i;

	RE_TEST_MACRO ("    Calling msiHiThere")

	i = hithere ();
	return(i);
}

