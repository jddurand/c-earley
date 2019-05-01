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

struct earleySymbol {
  int idi;
  int propertyBitSeti;
  int eventBitSeti;
};

struct earleyRule {
  int idi;
  int lshi;
  genericStack_t _rhsStack;
  genericStack_t *rhsStackp;
  int propertyBitSeti;
};

struct earleyGrammar {
  genericStack_t        _symbolStack;
  genericStack_t       *symbolStackp;
  genericStack_t        _ruleStack;
  genericStack_t       *ruleStackp;
  short                 precomputedb;
  earleyGrammarOption_t earleyGrammarOption;
};

#endif /* EARLEY_INTERNAL_STRUCTURES_H */
