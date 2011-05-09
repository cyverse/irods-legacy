/* For copyright information please refer to files in the COPYRIGHT directory
 */
#ifndef CACHE_H
#define CACHE_H
#include "rules.h"
#include "index.h"
#define CACHE_ENABLE 0

#define allocate(p, ty, vn, val) \
    ((CacheRecordDesc *)p)->type = CONCAT(ty,_T); \
    ((CacheRecordDesc *)p)->length = 1; \
    p+= sizeof(CacheRecordDesc); \
    ty *vn = ((ty *)p); \
    *vn = val; p+=sizeof(ty)
#define allocateArray(p, elemTy, n, lval, val) \
    ((CacheRecordDesc *)p)->type = CONCAT(elemTy,_T); \
    ((CacheRecordDesc *)p)->length = n; \
    p+= sizeof(CacheRecordDesc); \
    lval = ((elemTy *)p); \
    memcpy(lval, val, sizeof(elemTy)*(n)); \
    p+=sizeof(elemTy) * (n);
#define SHMMAX 30000000
#define SHM_BASE_ADDR ((void *)0x80000000)
#define APPLY_DIFF(p, t, d) if((p)!=NULL){unsigned char *temp = (unsigned char *)p; temp+=(d); (p)=(t *)temp;}
#define MAKE_COPY(buf, type, src, tgt) if((src)!=NULL) {tgt=CONCAT(copy, type)(&(buf), src);}
typedef struct {
    unsigned char *offset;
    long dataSize;
    RuleSet *coreRuleSet;
    Hashtable *funcDescIndex;
    Hashtable *coreRuleIndex;
    Hashtable *condIndex;
} Cache;
typedef void * (*Copier)(unsigned char **, void *, Hashtable *);
enum cacheRecordType {
        Cache_T,
        ExprType_T,
        ExprTypePtr_T,
        Node_T,
        NodePtr_T,
        Bucket_T,
        BucketPtr_T,
        Hashtable_T,
        RuleDesc_T,
        RuleSet_T,
        CondIndexVal_T,
        RuleIndexList_T,
        RuleIndexListNode_T,
        NodeType_T,
        char_T,
        int_T,
        FunctionDesc_T
    };

typedef struct {
    enum cacheRecordType type;
    int length;
} CacheRecordDesc;

typedef enum ruleEngineStatus {
    UNINITIALIZED,
    INITIALIZED
} RuleEngineStatus;

extern RuleEngineStatus _ruleEngineStatus;
extern int isServer;

RuleEngineStatus getRuleEngineStatus();
char *copyString(unsigned char **buf, char *string);
Node *copyNode(unsigned char **buf, Node *node, Hashtable *objectMap);
RuleDesc *copyRuleDesc(unsigned char **buf, RuleDesc *h, Hashtable *objectMap);
Hashtable* copyHashtableCharPrtToIntPtr(unsigned char **buf, Hashtable *h, Hashtable *objectMap);
RuleSet *copyRuleSet(unsigned char **buf, RuleSet *h, Hashtable *objectMap);
CondIndexVal *copyCondIndexVal(unsigned char **buf, CondIndexVal *civ, Hashtable *objectMap);
Cache *copyCache(unsigned char **buf, Cache *c);
int readRuleStructAndRuleSetFromFile(char *ruleBaseName, ruleStruct_t *inRuleStrct, Region *r);
int loadRuleFromCacheOrFile(char *irbSet, ruleStruct_t *inRuleStruct);

#endif
