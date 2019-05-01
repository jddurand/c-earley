#include <errno.h>
#include <stdlib.h>
#include <genericStack.h>

#include "earley/grammar.h"
#include "earley/internal/config.h"
#include "earley/internal/structures.h"

/****************************************************************************/
/* earleyGrammar_newp                                                       */
/****************************************************************************/
earleyGrammar_t *earleyGrammar_newp(earleyGrammarOption_t *earleyGrammarOptionp)
{
  earleyGrammar_t *earleyGrammarp;

  earleyGrammarp = (earleyGrammar_t *) malloc(sizeof(earleyGrammar_t));
  if (earleyGrammarp == NULL) {
    goto err;
  }

  earleyGrammarp->symbolStackp        = NULL;
  earleyGrammarp->ruleStackp          = NULL;
  earleyGrammarp->precomputedb        = 0;
  earleyGrammarp->earleyGrammarOption = (earleyGrammarOptionp != NULL) ? *earleyGrammarOptionp : earleyGrammarOptionDefault;

  earleyGrammarp->symbolStackp = &(earleyGrammarp->_symbolStack);
  GENERICSTACK_INIT(earleyGrammarp->symbolStackp);
  if (GENERICSTACK_ERROR(earleyGrammarp->symbolStackp)) {
    earleyGrammarp->symbolStackp = NULL;
    goto err;
  }

  earleyGrammarp->ruleStackp = &(earleyGrammarp->_ruleStack);
  GENERICSTACK_INIT(earleyGrammarp->ruleStackp);
  if (GENERICSTACK_ERROR(earleyGrammarp->ruleStackp)) {
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
earleyGrammar_t *earleyGrammar_clonep(earleyGrammar_t *earleyGrammarOriginp, earleyGrammarCloneOption_t *earleyGrammarCloneOptionp)
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
    errno = EINVAL;
    goto err;
  }

  earleyGrammarp = earleyGrammar_newp(NULL);
  if (earleyGrammarp == NULL) {
    goto err;
  }

  /* Duplicate symbol stack */
  symbolStackp = earleyGrammarp->symbolStackp;
  symbolOriginStackp = earleyGrammarOriginp->symbolStackp;
  for (i = 0; i < GENERICSTACK_USED(symbolOriginStackp); i++) {
    earleySymbolOriginp = (earleySymbol_t *) GENERICSTACK_GET_PTR(symbolOriginStackp, i);
    if (GENERICSTACK_ERROR(symbolOriginStackp)) {
      goto err;
    }
    if (earleySymbolOriginp == NULL) {
      errno = EINVAL;
      goto err;
    }
    earleySymbolp = (earleySymbol_t *) malloc(sizeof(earleySymbol_t));
    if (earleySymbolp == NULL) {
      goto err;
    }
    *earleySymbolp = *earleySymbolOriginp;
    GENERICSTACK_PUSH_PTR(symbolStackp, earleySymbolp);
    if (GENERICSTACK_ERROR(symbolStackp)) {
      goto err;
    }
  }

  /* Duplicate rule stack */
  ruleStackp = earleyGrammarp->ruleStackp;
  ruleOriginStackp = earleyGrammarOriginp->ruleStackp;
  for (i = 0; i < GENERICSTACK_USED(ruleOriginStackp); i++) {
    earleyRuleOriginp = (earleyRule_t *) GENERICSTACK_GET_PTR(ruleOriginStackp, i);
    if (GENERICSTACK_ERROR(ruleOriginStackp)) {
      goto err;
    }
    if (earleyRuleOriginp == NULL) {
      errno = EINVAL;
      goto err;
    }
    earleyRulep = (earleyRule_t *) malloc(sizeof(earleyRule_t));
    if (earleyRulep == NULL) {
      goto err;
    }
    *earleyRulep = *earleyRuleOriginp;
    GENERICSTACK_PUSH_PTR(ruleStackp, earleyRulep);
    if (GENERICSTACK_ERROR(ruleStackp)) {
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
/* earleyGrammar_clonep                                                     */
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
        if (earleySymbolp != NULL) {
          free(earleySymbolp);
        }
      }
      GENERICSTACK_RESET(symbolStackp);
    }

    /* Free rule stack */
    ruleStackp = earleyGrammarp->ruleStackp;
    if (ruleStackp != NULL) {
      for (i = 0; i < GENERICSTACK_USED(ruleStackp); i++) {
        earleyRulep = (earleyRule_t *) GENERICSTACK_GET_PTR(ruleStackp, i);
        if (earleyRulep != NULL) {
          free(earleyRulep);
        }
      }
      GENERICSTACK_RESET(ruleStackp);
    }

    free(earleyGrammarp);
  }
}

