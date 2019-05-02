#ifndef EARLEY_GRAMMAR_H
#define EARLEY_GRAMMAR_H

#include <stddef.h>
#include <stdarg.h>

#include <earley/export.h>
#include <genericLogger.h>

/* ---------------- */
/* Opaque structure */
/* ---------------- */
typedef struct earleyGrammar earleyGrammar_t;

/* ------------------------------------------ */
/* For every symbol there can be three events */
/* ------------------------------------------ */
typedef enum earleyGrammarEventType {
  EARLEYGRAMMAR_EVENTTYPE_NONE       = 0x00,
  EARLEYGRAMMAR_EVENTTYPE_COMPLETION = 0x01,
  EARLEYGRAMMAR_EVENTTYPE_NULLED     = 0x02,
  EARLEYGRAMMAR_EVENTTYPE_PREDICTION = 0x04
} earleyGrammarEventType_t;

/* Possible triggered events */
typedef struct earleyGrammarEvent {
  enum {
    EARLEYGRAMMAR_EVENT_COMPLETED,
    EARLEYGRAMMAR_EVENT_NULLED,
    EARLEYGRAMMAR_EVENT_EXPECTED,
    EARLEYGRAMMAR_EVENT_EXHAUSTED
  } eventType;
  int symboli; /* -1 in case of exhaustion, symbolId otherwise */
} earleyGrammarEvent_t;

/* ------------------ */
/* Options per symbol */
/* ------------------ */
typedef struct earleyGrammarSymbolOption {
  short  terminalb;             /* Default: 0. Eventually force symbol to be terminal         */
  short  startb;                /* Default: 0. Eventually force symbol to be the start symbol */
  int    eventSeti;             /* Default: EARLEYGRAMMAR_EVENTTYPE_NONE.               */
} earleyGrammarSymbolOption_t;

/* ---------------- */
/* Options per rule */
/* ---------------- */
typedef struct earleyGrammarRuleOption {
  int    ranki;          /* Default: 0. Rank                                        */
  short  nullRanksHighb; /* Default: 0. Null variant pattern                        */
  short  sequenceb;      /* Default: 0. Sequence ?                                  */
  int    separatorSymboli; /* Default: -1. Eventual separator symbol                */
  short  properb;        /* Default: 0. Proper flag                                 */
  int    minimumi;       /* Default: 0. Mininimum - must be 0 or 1                  */
} earleyGrammarRuleOption_t;

/* --------------- */
/* General options */
/* --------------- */
typedef struct earleyGrammarOption {
  genericLogger_t *genericLoggerp;             /* Default: NULL.                                      */
  short            warningIsErrorb;            /* Default: 0. Have precedence over warningIsIgnoredb  */
  short            warningIsIgnoredb;          /* Default: 0.                                         */
  short            autorankb;                  /* Default: 0.                                         */
} earleyGrammarOption_t;

typedef enum earleySymbolProperty {
  EARLEY_SYMBOL_IS_ACCESSIBLE = 0x01,
  EARLEY_SYMBOL_IS_NULLABLE   = 0x02,
  EARLEY_SYMBOL_IS_NULLING    = 0x04,
  EARLEY_SYMBOL_IS_PRODUCTIVE = 0x08,
  EARLEY_SYMBOL_IS_START      = 0x10,
  EARLEY_SYMBOL_IS_TERMINAL   = 0x20
} earleySymbolProperty_t;

typedef enum earleyRuleProperty {
  EARLEY_RULE_IS_ACCESSIBLE = 0x01,
  EARLEY_RULE_IS_NULLABLE   = 0x02,
  EARLEY_RULE_IS_NULLING    = 0x04,
  EARLEY_RULE_IS_LOOP       = 0x08,
  EARLEY_RULE_IS_PRODUCTIVE = 0x10
} earleyRuleProperty_t;

typedef short (*earleyGrammar_grammarOptionSetter_t)(void *userDatavp, earleyGrammarOption_t *earleyGrammarOptionp);
typedef short (*earleyGrammar_symbolOptionSetter_t)(void *userDatavp, int symboli, earleyGrammarSymbolOption_t *earleyGrammarSymbolOptionp);
typedef short (*earleyGrammar_ruleOptionSetter_t)(void *userDatavp, int rulei, earleyGrammarRuleOption_t *earleyGrammarRuleOptionp);

typedef struct earleyGrammarCloneOption {
  void                                *userDatavp;           /* Default: NULL. User context */
  earleyGrammar_grammarOptionSetter_t  grammarOptionSetterp; /* Default: NULL. Overwrite grammar option */
  earleyGrammar_symbolOptionSetter_t   symbolOptionSetterp;  /* Default: NULL. Overwrite event symbol option */
  earleyGrammar_ruleOptionSetter_t     ruleOptionSetterp;    /* Default: NULL. Overwrite event rule option */
} earleyGrammarCloneOption_t;

#ifdef __cplusplus
extern "C" {
#endif
  earley_EXPORT earleyGrammar_t *earleyGrammar_newp(earleyGrammarOption_t *earleyGrammarOptionp);
  earley_EXPORT earleyGrammar_t *earleyGrammar_clonep(earleyGrammar_t *earleyGrammarOriginp, earleyGrammarCloneOption_t *earleyGrammarCloneOptionp);
  earley_EXPORT void             earleyGrammar_freev(earleyGrammar_t *earleyGrammarp);

  earley_EXPORT short            earleyGrammar_errorb(earleyGrammar_t *earleyGrammarp, int *errorip);
  earley_EXPORT short            earleyGrammar_error_clearb(earleyGrammar_t *earleyGrammarp);

  earley_EXPORT int              earleyGrammar_newSymboli(earleyGrammar_t *earleyGrammarp, earleyGrammarSymbolOption_t *earleyGrammarSymbolOptionp);
  earley_EXPORT short            earleyGrammar_symbolPropertyb(earleyGrammar_t *earleyGrammarp, int symboli, int *earleySymbolPropertyBitSetp);
  earley_EXPORT short            earleyGrammar_symbolEventb(earleyGrammar_t *earleyGrammarp, int symboli, int *earleySymbolEventBitSetp);
  earley_EXPORT int              earleyGrammar_newRulei(earleyGrammar_t *earleyGrammarp, earleyGrammarRuleOption_t *earleyGrammarRuleOptionp,
										int lhsSymboli,
										size_t rhsSymboll, int *rhsSymbolip
										);
  earley_EXPORT short            earleyGrammar_rulePropertyb(earleyGrammar_t *earleyGrammarp, int rulei, int *earleyRulePropertyBitSetp);
  /* Handy methods to create symbols and rules that I find more user-friendly */
  earley_EXPORT int              earleyGrammar_newSymbolExti(earleyGrammar_t *earleyGrammarp, short terminalb, short startb, int eventSeti);
  earley_EXPORT int              earleyGrammar_newRuleExti(earleyGrammar_t *earleyGrammarp, int ranki, short nullRanksHighb, int lhsSymboli, ...);
  earley_EXPORT int              earleyGrammar_newSequenceExti(earleyGrammar_t *earleyGrammarp, int ranki, short nullRanksHighb,
										       int lhsSymboli,
										       int rhsSymboli, int minimumi, int separatorSymboli, short properb);
  
  earley_EXPORT short            earleyGrammar_precomputeb(earleyGrammar_t *earleyGrammarp);
  earley_EXPORT short            earleyGrammar_precompute_startb(earleyGrammar_t *earleyGrammarp, int starti);
  earley_EXPORT short            earleyGrammar_eventb(earleyGrammar_t *earleyGrammarp, size_t *eventlp, earleyGrammarEvent_t **eventpp, short exhaustionEventb, short forceReloadb);
#ifdef __cplusplus
}
#endif

/* Very often, symbols and rules are created with no particular attribute */
/* These macros are just short-hands to make life easier.                 */
#define EARLEYGRAMMAR_NEWSYMBOL(earleyGrammarp) earleyGrammar_newSymboli((earleyGrammarp), NULL)
#define EARLEYGRAMMAR_NEWRULE(earleyGrammarp, lhsSymboli, ...) earleyGrammar_newRuleExti((earleyGrammarp), 0, 0, (lhsSymboli), __VA_ARGS__)
#define EARLEYGRAMMAR_NEWSEQUENCE(earleyGrammarp, lhsSymboli, rhsSymboli, minimumi) earleyGrammar_newSequenceExti((earleyGrammarp), 0, 0, (lhsSymboli), (rhsSymboli), (minimumi), -1, 0)

#endif /* EARLEY_GRAMMAR_H */
