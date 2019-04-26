#ifndef EARLEY_H
#define EARLEY_H

#include <stddef.h> /* For size_t */

/* Forward declarations */
typedef enum   earley_symbol_property  earley_symbol_property_t;
typedef enum   earley_symbol_event     earley_symbol_event_t;
typedef struct earley_symbol           earley_symbol_t;

typedef enum   earley_rule_property    earley_rule_property_t;
typedef enum   earley_rule_event       earley_rule_event_t;
typedef struct earley_rule             earley_rule_t;

typedef struct earley_grammar          earley_grammar_t;

/* Symbol properties */
enum earley_symbol_property {
  EARLEY_SYMBOL_PROPERTY_ACCESSIBLE = 0x01,
  EARLEY_SYMBOL_PROPERTY_NULLABLE   = 0x02,
  EARLEY_SYMBOL_PROPERTY_NULLING    = 0x04,
  EARLEY_SYMBOL_PROPERTY_PRODUCTIVE = 0x08,
  EARLEY_SYMBOL_PROPERTY_START      = 0x10,
  EARLEY_SYMBOL_PROPERTY_TERMINAL   = 0x20
};

/* Symbol events */
enum earley_symbol_event {
  EARLEY_SYMBOL_EVENT_COMPLETED  = 0x01,
  EARLEY_SYMBOL_EVENT_NULLED     = 0x02,
  EARLEY_SYMBOL_EVENT_PREDICTED  = 0x04
};

/* A symbol has properties, events and is part of rules */
struct earley_symbol {
  earley_symbol_property_t propertyBitSet;
  earley_symbol_event_t    eventBitSet;
  earley_rule_t           *ruleArrayp;
  size_t                   ruleArrayl;
};

/* Rule properties */
enum earley_rule_property {
  EARLEY_RULE_PROPERTY_ACCESSIBLE = 0x01,
  EARLEY_RULE_PROPERTY_NULLABLE   = 0x02,
  EARLEY_RULE_PROPERTY_NULLING    = 0x04,
  EARLEY_RULE_PROPERTY_LOOP       = 0x08,
  EARLEY_RULE_PROPERTY_PRODUCTIVE = 0x10
};

/* Rule events */
enum earley_rule_event {
  EARLEY_RULE_EVENT_COMPLETED  = 0x01,
  EARLEY_RULE_EVENT_PREDICTED  = 0x02
};

/* A rule has properties, events, is a symbol on the left (LHS), symbols on the right (RHSs) */
struct earley_rule {
  earley_rule_property_t  propertyBitSet;
  earley_rule_event_t     eventBitSet;
  earley_symbol_t        *lhsp;
  earley_symbol_t        *rhsArrayp;
  size_t                  rhsArrayl;
};

/* A grammar is made of symbols and rules, has a start rule */
struct earley_grammar {
  earley_symbol_t           *symbolArrayp;
  size_t                     symbolArrayl;
  earley_rule_t             *ruleArrayp;
  size_t                     ruleArrayl;
  int                        rulestarti;
};

#endif /* EARLEY_H */
