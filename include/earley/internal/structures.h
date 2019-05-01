#ifndef EARLEY_INTERNAL_STRUCTURES_H
#define EARLEY_INTERNAL_STRUCTURES_H

#include <genericStack.h>

#include "earley.h"

typedef struct earleySymbol earleySymbol_t;
typedef struct earleyRule   earleyRule_t;

earleyGrammarOption_t earleyGrammarOptionDefault = {
  0, /* warningIsErrorb */
  0, /* warningIsIgnoredb */
  0  /* autorankb */
};

earleyGrammarCloneOption_t earleyGrammarCloneOptionDefault = {
  NULL, /* userDatavp */
  NULL, /* grammarOptionSetterp */
  NULL, /* symbolOptionSetterp */
  NULL /* ruleOptionSetterp */
};

struct earleySymbol {
  int                         idi;
  int                         propertyBitSeti;
  int                         eventBitSeti;
  earleyGrammarSymbolOption_t option;
};

struct earleyRule {
  int                       idi;
  int                       lshi;
  genericStack_t            _rhsStack;
  genericStack_t            *rhsStackp;
  int                        propertyBitSeti;
  earleyGrammarRuleOption_t  option;
};

struct earleyGrammar {
  genericStack_t        _symbolStack;
  genericStack_t       *symbolStackp;
  genericStack_t        _ruleStack;
  genericStack_t       *ruleStackp;
  short                 precomputedb;
  earleyGrammarOption_t option;
};

#endif /* EARLEY_INTERNAL_STRUCTURES_H */
