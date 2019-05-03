#include <errno.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <genericLogger.h>
#include <genericStack.h>

#include "earley/grammar.h"
#include "earley/internal/config.h"
#include "earley/internal/structures.h"

static inline earleySymbol_t *earleySymbol_getp(earleyGrammar_t *earleyGrammarp, int symboli);
static inline earleyRule_t   *earleyRule_getp(earleyGrammar_t *earleyGrammarp, int rulei);
static inline void            earleySymbol_freev(earleySymbol_t *earleySymbolp);
static inline void            earleyRule_freev(earleyRule_t *earleyRulep);

#define EARLEYGRAMMAR_ERROR(earleyGrammarp, strings) do {               \
    if ((earleyGrammarp != NULL) && (earleyGrammarp->option.genericLoggerp != NULL)) { \
      GENERICLOGGER_ERROR(earleyGrammarp->option.genericLoggerp, strings); \
    }                                                                   \
  } while (0)

#define EARLEYGRAMMAR_ERRORF(earleyGrammarp, fmts, ...) do {            \
    if ((earleyGrammarp != NULL) && (earleyGrammarp->option.genericLoggerp != NULL)) { \
      GENERICLOGGER_ERRORF(earleyGrammarp->option.genericLoggerp, fmts, __VA_ARGS__); \
    }                                                                   \
  } while (0)

/****************************************************************************/
static inline earleySymbol_t *earleySymbol_getp(earleyGrammar_t *earleyGrammarp, int symboli)
/****************************************************************************/
{
  genericStack_t *symbolStackp;
  earleySymbol_t *earleySymbolp;

  if (earleyGrammarp == NULL) {
    errno = EINVAL;
    goto err;
  }

  symbolStackp = earleyGrammarp->symbolStackp;

  if (! GENERICSTACK_IS_PTR(symbolStackp, symboli)) {
    EARLEYGRAMMAR_ERRORF(earleyGrammarp, "No such symbol %d\n", symboli);
    errno = ENOENT;
    goto err;
  }

  earleySymbolp = GENERICSTACK_GET_PTR(symbolStackp, symboli);
  if (GENERICSTACK_ERROR(symbolStackp)) {
    EARLEYGRAMMAR_ERRORF(earleyGrammarp, "GENERICSTACK_GET_PTR failure, %s\n", strerror(errno));
    goto err;
  }

  goto done;

 err:
  earleySymbolp = NULL;

 done:
  return earleySymbolp;
}

/****************************************************************************/
static inline earleyRule_t *earleyRule_getp(earleyGrammar_t *earleyGrammarp, int rulei)
/****************************************************************************/
{
  genericStack_t *ruleStackp;
  earleyRule_t   *earleyRulep;

  if (earleyGrammarp == NULL) {
    errno = EINVAL;
    goto err;
  }

  ruleStackp = earleyGrammarp->ruleStackp;

  if (! GENERICSTACK_IS_PTR(ruleStackp, rulei)) {
    EARLEYGRAMMAR_ERRORF(earleyGrammarp, "No such rule %d\n", rulei);
    errno = ENOENT;
    goto err;
  }

  earleyRulep = GENERICSTACK_GET_PTR(ruleStackp, rulei);
  if (GENERICSTACK_ERROR(ruleStackp)) {
    EARLEYGRAMMAR_ERRORF(earleyGrammarp, "GENERICSTACK_GET_PTR failure, %s\n", strerror(errno));
    goto err;
  }

  goto done;

 err:
  earleyRulep = NULL;

 done:
  return earleyRulep;
}

/****************************************************************************/
static inline void earleySymbol_freev(earleySymbol_t *earleySymbolp)
/****************************************************************************/
{
  if (earleySymbolp != NULL) {
    free(earleySymbolp);
  }
}

/****************************************************************************/
static inline void earleyRule_freev(earleyRule_t *earleyRulep)
/****************************************************************************/
{
  if (earleyRulep != NULL) {
    if (earleyRulep->rhsStackp != NULL) {
      GENERICSTACK_RESET(earleyRulep->rhsStackp);
    }
    free(earleyRulep);
  }
}

/****************************************************************************/
/* earleyGrammar_newp                                                       */
/****************************************************************************/
earleyGrammar_t *earleyGrammar_newp(earleyGrammarOption_t *optionp)
{
  earleyGrammar_t *earleyGrammarp;
  genericLogger_t *genericLoggerp;

  if (optionp == NULL) {
    optionp = &earleyGrammarOptionDefault;
  }

  genericLoggerp = optionp->genericLoggerp;

  earleyGrammarp = (earleyGrammar_t *) malloc(sizeof(earleyGrammar_t));
  if (earleyGrammarp == NULL) {
    GENERICLOGGER_ERRORF(genericLoggerp, "malloc failure: %s", strerror(errno));
    goto err;
  }

  earleyGrammarp->symbolStackp = NULL;
  earleyGrammarp->ruleStackp   = NULL;
  earleyGrammarp->errori       = 0;
  earleyGrammarp->option       = *optionp;
  earleyGrammarp->precomputedb = 0;

  earleyGrammarp->symbolStackp = &(earleyGrammarp->_symbolStack);
  GENERICSTACK_INIT(earleyGrammarp->symbolStackp);
  if (GENERICSTACK_ERROR(earleyGrammarp->symbolStackp)) {
    EARLEYGRAMMAR_ERRORF(earleyGrammarp, "GENERICSTACK_INIT failure, %s\n", strerror(errno));
    earleyGrammarp->symbolStackp = NULL;
    goto err;
  }

  earleyGrammarp->ruleStackp = &(earleyGrammarp->_ruleStack);
  GENERICSTACK_INIT(earleyGrammarp->ruleStackp);
  if (GENERICSTACK_ERROR(earleyGrammarp->ruleStackp)) {
    EARLEYGRAMMAR_ERRORF(earleyGrammarp, "GENERICSTACK_INIT failure, %s\n", strerror(errno));
    earleyGrammarp->ruleStackp = NULL;
    goto err;
  }

  goto done;

 err:
  earleyGrammar_freev(earleyGrammarp);
  earleyGrammarp = NULL;

 done:
  return earleyGrammarp;
}

/****************************************************************************/
/* earleyGrammar_clonep                                                     */
/****************************************************************************/
earleyGrammar_t *earleyGrammar_clonep(earleyGrammar_t *earleyGrammarOriginp, earleyGrammarCloneOption_t *optionp)
{
  earleyGrammar_t *earleyGrammarp = NULL;
  genericStack_t  *symbolStackp;
  genericStack_t  *ruleStackp;
  earleySymbol_t  *earleySymbolp;
  earleyRule_t    *earleyRulep;
  genericStack_t  *symbolOriginStackp;
  genericStack_t  *ruleOriginStackp;
  earleySymbol_t  *earleySymbolOriginp;
  earleyRule_t    *earleyRuleOriginp;
  int              i;

  if (earleyGrammarOriginp == NULL) {
    errno = EINVAL;
    goto err;
  }

  /* Only precomputed grammars can be cloned */
  if (! earleyGrammarOriginp->precomputedb) {
    EARLEYGRAMMAR_ERROR(earleyGrammarOriginp, "Grammar must be precomputed\n");
    errno = EINVAL;
    goto err;
  }

  earleyGrammarp = earleyGrammar_newp(NULL);
  if (earleyGrammarp == NULL) {
    EARLEYGRAMMAR_ERRORF(earleyGrammarOriginp, "malloc failure, %s\n", strerror(errno));
    goto err;
  }

  /* Duplicate symbol stack */
  symbolStackp = earleyGrammarp->symbolStackp;
  symbolOriginStackp = earleyGrammarOriginp->symbolStackp;
  for (i = 0; i < GENERICSTACK_USED(symbolOriginStackp); i++) {
    earleySymbolOriginp = earleySymbol_getp(earleyGrammarOriginp, i);
    if (earleySymbolOriginp == NULL) {
      goto err;
    }
    earleySymbolp = (earleySymbol_t *) malloc(sizeof(earleySymbol_t));
    if (earleySymbolp == NULL) {
      EARLEYGRAMMAR_ERRORF(earleyGrammarOriginp, "malloc failure, %s\n", strerror(errno));
      goto err;
    }
    *earleySymbolp = *earleySymbolOriginp;
    GENERICSTACK_PUSH_PTR(symbolStackp, earleySymbolp);
    if (GENERICSTACK_ERROR(symbolStackp)) {
      EARLEYGRAMMAR_ERRORF(earleyGrammarOriginp, "GENERICSTACK_PUSH_PTR failure, %s\n", strerror(errno));
      goto err;
    }
    /* Apply clone options */
    if (optionp != NULL) {
      if (! optionp->symbolOptionSetterp(optionp->userDatavp, earleySymbolp->idi, &(earleySymbolp->option))) {
        EARLEYGRAMMAR_ERROR(earleyGrammarOriginp, "symbolOptionSetterp failure\n");
        goto err;
      }
    }
  }

  /* Duplicate rule stack */
  ruleStackp = earleyGrammarp->ruleStackp;
  ruleOriginStackp = earleyGrammarOriginp->ruleStackp;
  for (i = 0; i < GENERICSTACK_USED(ruleOriginStackp); i++) {
    earleyRuleOriginp = earleyRule_getp(earleyGrammarOriginp, i);
    if (earleyRuleOriginp == NULL) {
      goto err;
    }
    earleyRulep = (earleyRule_t *) malloc(sizeof(earleyRule_t));
    if (earleyRulep == NULL) {
      EARLEYGRAMMAR_ERRORF(earleyGrammarOriginp, "malloc failure, %s\n", strerror(errno));
      goto err;
    }
    *earleyRulep = *earleyRuleOriginp;
    GENERICSTACK_PUSH_PTR(ruleStackp, earleyRulep);
    if (GENERICSTACK_ERROR(ruleStackp)) {
      EARLEYGRAMMAR_ERRORF(earleyGrammarOriginp, "GENERICSTACK_PUSH_PTR failure, %s\n", strerror(errno));
      goto err;
    }
    /* Apply clone options */
    if (optionp != NULL) {
      if (! optionp->ruleOptionSetterp(optionp->userDatavp, earleyRulep->idi, &(earleyRulep->option))) {
        EARLEYGRAMMAR_ERROR(earleyGrammarOriginp, "ruleOptionSetterp failure\n");
        goto err;
      }
    }
  }

  /* Apply clone options */
  if (optionp != NULL) {
    if (! optionp->grammarOptionSetterp(optionp->userDatavp, &(earleyGrammarp->option))) {
      EARLEYGRAMMAR_ERROR(earleyGrammarOriginp, "grammarOptionSetterp failure\n");
      goto err;
    }
  }

  goto done;

 err:
  earleyGrammar_freev(earleyGrammarp);
  earleyGrammarp = NULL;

 done:
  return earleyGrammarp;
}

/****************************************************************************/
/* earleyGrammar_freev                                                      */
/****************************************************************************/
void earleyGrammar_freev(earleyGrammar_t *earleyGrammarp)
{
  genericStack_t *symbolStackp;
  genericStack_t *ruleStackp;
  earleySymbol_t *earleySymbolp;
  earleyRule_t   *earleyRulep;
  int             i;

  if (earleyGrammarp != NULL) {

    /* Free symbol stack */
    symbolStackp = earleyGrammarp->symbolStackp;
    if (symbolStackp != NULL) {
      for (i = 0; i < GENERICSTACK_USED(symbolStackp); i++) {
        earleySymbolp = (earleySymbol_t *) GENERICSTACK_GET_PTR(symbolStackp, i);
        earleySymbol_freev(earleySymbolp);
      }
      GENERICSTACK_RESET(symbolStackp);
    }

    /* Free rule stack */
    ruleStackp = earleyGrammarp->ruleStackp;
    if (ruleStackp != NULL) {
      for (i = 0; i < GENERICSTACK_USED(ruleStackp); i++) {
        earleyRulep = (earleyRule_t *) GENERICSTACK_GET_PTR(ruleStackp, i);
        earleyRule_freev(earleyRulep);
      }
      GENERICSTACK_RESET(ruleStackp);
    }

    free(earleyGrammarp);
  }
}

/****************************************************************************/
int earleyGrammar_newSymboli(earleyGrammar_t *earleyGrammarp, earleyGrammarSymbolOption_t *optionp)
/****************************************************************************/
{
  earleySymbol_t *earleySymbolp = NULL;
  genericStack_t *symbolStackp;
  int             symboli;

  if (earleyGrammarp == NULL) {
    errno = EINVAL;
    goto err;
  }

  symbolStackp = earleyGrammarp->symbolStackp;

  earleySymbolp = (earleySymbol_t *) malloc(sizeof(earleySymbol_t));
  if (earleySymbolp == NULL) {
    EARLEYGRAMMAR_ERRORF(earleyGrammarp, "malloc failure, %s\n", strerror(errno));
    goto err;
  }

  earleySymbolp->idi             = GENERICSTACK_USED(symbolStackp);
  earleySymbolp->propertyBitSeti = 0;
  earleySymbolp->eventBitSeti    = 0;
  earleySymbolp->option          = (optionp != NULL) ? *optionp : earleyGrammarSymbolOptionDefault;

  GENERICSTACK_PUSH_PTR(symbolStackp, earleySymbolp);
  if (GENERICSTACK_ERROR(symbolStackp)) {
    EARLEYGRAMMAR_ERRORF(earleyGrammarp, "GENERICSTACK_PUSH_PTR failure, %s\n", strerror(errno));
    goto err;
  }

  symboli = earleySymbolp->idi;
  goto done;

 err:
  earleySymbol_freev(earleySymbolp);
  symboli = -1;

 done:
  return symboli;
}

/****************************************************************************/
short earleyGrammar_symbolPropertyb(earleyGrammar_t *earleyGrammarp, int symboli, int *earleySymbolPropertyBitSetp)
/****************************************************************************/
{
  earleySymbol_t *earleySymbolp;
  short           rcb;

  if (earleyGrammarp == NULL) {
    errno = EINVAL;
    goto err;
  }

  earleySymbolp = earleySymbol_getp(earleyGrammarp, symboli);
  if (earleySymbolp == NULL) {
    goto err;
  }

  if (earleySymbolPropertyBitSetp != NULL) {
    *earleySymbolPropertyBitSetp = earleySymbolp->propertyBitSeti;
  }

  rcb = 1;
  goto done;

 err:
  rcb = 0;

 done:
  return rcb;
}

/****************************************************************************/
short earleyGrammar_symbolEventb(earleyGrammar_t *earleyGrammarp, int symboli, int *earleySymbolEventBitSetp)
/****************************************************************************/
{
  earleySymbol_t *earleySymbolp;
  short           rcb;

  if (earleyGrammarp == NULL) {
    errno = EINVAL;
    goto err;
  }

  earleySymbolp = earleySymbol_getp(earleyGrammarp, symboli);
  if (earleySymbolp == NULL) {
    goto err;
  }

  if (earleySymbolEventBitSetp != NULL) {
    *earleySymbolEventBitSetp = earleySymbolp->eventBitSeti;
  }

  rcb = 1;
  goto done;

 err:
  rcb = 0;

 done:
  return rcb;
}

/****************************************************************************/
int earleyGrammar_newRulei(earleyGrammar_t *earleyGrammarp, earleyGrammarRuleOption_t *optionp,
                           int lhsSymboli,
                           size_t rhsSymboll, int *rhsSymbolip
                           )
/****************************************************************************/
{
  earleyRule_t   *earleyRulep = NULL;
  earleySymbol_t *earleySymbolp;
  genericStack_t *rhsStackp;
  genericStack_t *ruleStackp;
  int             rulei;
  size_t          l;

  if (earleyGrammarp == NULL) {
    errno = EINVAL;
    goto err;
  }

  ruleStackp   = earleyGrammarp->ruleStackp;

  earleyRulep = (earleyRule_t *) malloc(sizeof(earleyRule_t));
  if (earleyRulep == NULL) {
    EARLEYGRAMMAR_ERRORF(earleyGrammarp, "malloc failure, %s\n", strerror(errno));
    goto err;
  }

  earleyRulep->idi             = GENERICSTACK_USED(ruleStackp);
  earleyRulep->lshSymbolp      = NULL;
  earleyRulep->rhsStackp       = NULL;
  earleyRulep->propertyBitSeti = 0;
  earleyRulep->option          = (optionp != NULL) ? *optionp : earleyGrammarRuleOptionDefault;

  rhsStackp = &(earleyRulep->_rhsStack);
  GENERICSTACK_INIT(rhsStackp);
  if (GENERICSTACK_ERROR(rhsStackp)) {
    EARLEYGRAMMAR_ERRORF(earleyGrammarp, "GENERICSTACK_INIT failure, %s\n", strerror(errno));
    goto err;
  }
  earleyRulep->rhsStackp = rhsStackp;

  earleyRulep->lshSymbolp = earleySymbol_getp(earleyGrammarp, lhsSymboli);
  if (earleyRulep->lshSymbolp == NULL) {
    goto err;
  }
  for (l = 0; l < rhsSymboll; l++) {
    earleySymbolp = earleySymbol_getp(earleyGrammarp, rhsSymbolip[l]);
    if (earleySymbolp == NULL) {
      goto err;
    }
    GENERICSTACK_PUSH_PTR(rhsStackp, earleySymbolp);
    if (GENERICSTACK_ERROR(rhsStackp)) {
      EARLEYGRAMMAR_ERRORF(earleyGrammarp, "GENERICSTACK_PUSH_PTR failure, %s\n", strerror(errno));
      goto err;
    }
  }

  rulei = earleySymbolp->idi;
  goto done;

 err:
  earleyRule_freev(earleyRulep);
  rulei = -1;

 done:
  return rulei;
}

/****************************************************************************/
short earleyGrammar_rulePropertyb(earleyGrammar_t *earleyGrammarp, int rulei, int *earleyRulePropertyBitSetp)
/****************************************************************************/
{
  earleyRule_t *earleyRulep;
  short           rcb;

  if (earleyGrammarp == NULL) {
    errno = EINVAL;
    goto err;
  }

  earleyRulep = earleyRule_getp(earleyGrammarp, rulei);
  if (earleyRulep == NULL) {
    goto err;
  }

  if (earleyRulePropertyBitSetp != NULL) {
    *earleyRulePropertyBitSetp = earleyRulep->propertyBitSeti;
  }

  rcb = 1;
  goto done;

 err:
  rcb = 0;

 done:
  return rcb;
}

/****************************************************************************/
int earleyGrammar_newSymbolExti(earleyGrammar_t *earleyGrammarp, short terminalb, short startb, int eventSeti)
/****************************************************************************/
{
  earleyGrammarSymbolOption_t option = earleyGrammarSymbolOptionDefault;

  option.terminalb = terminalb;
  option.startb    = startb;
  option.eventSeti = eventSeti;

  return earleyGrammar_newSymboli(earleyGrammarp, &option);
}

/****************************************************************************/
int earleyGrammar_newRuleExti(earleyGrammar_t *earleyGrammarp, int ranki, short nullRanksHighb, int lhsSymboli, ...)
/****************************************************************************/
{
  earleyGrammarRuleOption_t option               = earleyGrammarRuleOptionDefault;
  static const size_t       rhsSymbolStartAllocl = 16; /* Totally subjective -; */
  size_t                    rhsSymbolAllocl      = 0;
  size_t                    rhsSymboll           = 0;
  size_t                    l                    = 0;
  int                      *rhsSymbolip          = NULL;
  int                      *tmpip;
  int                       rhsSymboli;
  size_t                    tmpl;
  va_list                   ap;
  int                       rci;

  option.ranki          = ranki;
  option.nullRanksHighb = nullRanksHighb;

  va_start(ap, lhsSymboli);
  while ((rhsSymboli = va_arg(ap, int)) >= 0) {
    if (++rhsSymboll > rhsSymbolAllocl) {
      if (rhsSymbolAllocl == 0) {
        rhsSymbolip = (int *) malloc(rhsSymbolStartAllocl * sizeof(int));
        if (rhsSymbolip == NULL) {
          EARLEYGRAMMAR_ERRORF(earleyGrammarp, "malloc failure, %s\n", strerror(errno));
          goto err;
        }
        rhsSymbolAllocl = rhsSymbolStartAllocl;
      } else {
        tmpl = rhsSymbolAllocl * 2;
        /* Detect very improbable turnaround */
        if (tmpl < rhsSymbolAllocl) {
          EARLEYGRAMMAR_ERROR(earleyGrammarp, "size_t turnaround\n");
          errno = EINVAL;
          goto err;
        }
        tmpip = (int *) realloc(rhsSymbolip, tmpl * sizeof(int));
        if (tmpip == NULL) {
          EARLEYGRAMMAR_ERRORF(earleyGrammarp, "realloc failure, %s\n", strerror(errno));
          goto err;
        }
        rhsSymbolip = tmpip;
        rhsSymbolAllocl = tmpl;
      }
    }

    rhsSymbolip[l++] = rhsSymboli;
  }
  va_end(ap);

  rci =  earleyGrammar_newRulei(earleyGrammarp, &option, lhsSymboli, rhsSymboll, rhsSymbolip);

  goto done;

 err:
  rci = -1;

 done:
  if (rhsSymbolip != NULL) {
    free(rhsSymbolip);
  }

  return rci;
}

/****************************************************************************/
int earley_newSequenceExti(earleyGrammar_t *earleyGrammarp, int ranki, short nullRanksHighb,
                           int lhsSymboli,
                           int rhsSymboli, int minimumi, int separatorSymboli, short properb)
/****************************************************************************/
{
  earleyGrammarRuleOption_t option;
  int                       rhsSymbolip[1] = { rhsSymboli };

  switch (minimumi) {
  case '*':
    minimumi = 0;
    break;
  case '+':
    minimumi = 1;
    break;
  default:
    break;
  }
  
  option.ranki            = ranki;
  option.nullRanksHighb   = nullRanksHighb;
  option.sequenceb        = 1;
  option.separatorSymboli = separatorSymboli;
  option.properb          = properb;
  option.minimumi         = minimumi;

  return earleyGrammar_newRulei(earleyGrammarp, &option, lhsSymboli, 1, rhsSymbolip);
}

/****************************************************************************/
short earleyGrammar_errorb(earleyGrammar_t *earleyGrammarp, int *errorip)
/****************************************************************************/
{
  short rcb;

  if (earleyGrammarp == NULL) {
    errno = EINVAL;
    goto err;
  }

  if (errorip != NULL) {
    *errorip = earleyGrammarp->errori;
  }

  rcb = 1;
  goto done;

 err:
  rcb = 0;

 done:
  return rcb;
}

/****************************************************************************/
short earleyGrammar_error_clearb(earleyGrammar_t *earleyGrammarp)
/****************************************************************************/
{
  short rcb;

  if (earleyGrammarp == NULL) {
    errno = EINVAL;
    goto err;
  }

  earleyGrammarp->errori = 0;

  rcb = 1;
  goto done;

 err:
  rcb = 0;

 done:
  return rcb;
}

/****************************************************************************/
short earleyGrammar_precomputeb(earleyGrammar_t *earleyGrammarp)
/****************************************************************************/
{
  return earleyGrammar_precompute_startb(earleyGrammarp, 0);
}

/****************************************************************************/
short earleyGrammar_precompute_startb(earleyGrammar_t *earleyGrammarp, int starti)
/****************************************************************************/
{
  /* TO DO */
  return 0;
}

/****************************************************************************/
short earleyGrammar_eventb(earleyGrammar_t *earleyGrammarp, size_t *eventlp, earleyGrammarEvent_t **eventpp, short exhaustionEventb, short forceReladb)
/****************************************************************************/
{
  /* TO DO */
  return 0;
}
