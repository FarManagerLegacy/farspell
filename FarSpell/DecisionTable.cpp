/*
    Run-time decision table engine.
    Copyright (c) Sergey Shishmintzev, Kiev 2006

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    Contributor(s):
      Sergey Shishmintzev <sergey.shishmintzev@gmail.com>
*/

#include <malloc.h>
#include <stdio.h>
#include "DecisionTable.hpp"

DecisionTable::action_t DecisionTable::noaction = -1;
DecisionTable::condition_id_t DecisionTable::nocondition_id = -1;

DecisionTable::~DecisionTable()
{
  delete[] _conditions;
  delete[] actions;
  delete[] condition_ids;
  for (unsigned r = 0; r<n_rules; r++)
    delete[] rules[r];
  delete[] rules;
}

DecisionTable::DecisionTable()
{
  rules = NULL;
  actions  = NULL;
  condition_ids = NULL;
  n_conditions = n_rules = 0;
  _conditions = NULL;
  LastError = Ok;
}

DecisionTable::DecisionTable(const FarString &source)
{
  unsigned i, r, c;
  FarStringTokenizer tokenizer (source, '|');

  rules = NULL;
  actions  = NULL;
  condition_ids = NULL;
  n_conditions = 0;
  n_rules = 0;
  _conditions = NULL;
  
  LastError = Ok;

  for (i = 0; i<2 && tokenizer.HasNext(); i++) 
    switch (i)
    {
      case 0: n_conditions = atoi(tokenizer.NextToken()); break;
      case 1: n_rules = atoi(tokenizer.NextToken()); break;
    }
  if (i!=2) {
    n_conditions = 0;
    n_rules = 0;
    LastError = InvalidSource;
    return;
  }
  if (n_conditions == 0) { n_rules = 0; return; }
  if (n_rules == 0) { n_conditions = 0; return; }

  Allocate(n_rules, n_conditions);

  for (c = 0; c<n_conditions && tokenizer.HasNext(); c++)  
    condition_ids[c] = atoi(tokenizer.NextToken());
  if (c!=n_conditions) {
    LastError = InvalidSource;
    return;
  }

  for (r = 0; r<n_rules && tokenizer.HasNext(); r++)  {
    int coded_rule = atoi(tokenizer.NextToken());
    condition_t *rule = rules[r];
    for (unsigned c = 0; c<n_conditions; (c++),(coded_rule>>=2)) 
      rule[c] = (condition_t)(coded_rule&3);
    if (!tokenizer.HasNext()) {
      LastError = InvalidSource;
      return;
    }
    actions[r] = atoi(tokenizer.NextToken());
  }
  if (r!=n_rules) {
    LastError = InvalidSource;
    return;
  }
}

FarString DecisionTable::AsString() 
{
  FarString s;
  char buf[128];

  LastError = Ok;

  sprintf(buf, "%u|%u|", n_conditions, n_rules);
  s = buf;
  if (n_conditions==0 || n_rules==0) return s;
  for (unsigned c = 0; c<n_conditions; c++) {
    sprintf(buf, "%u|", condition_ids[c]);
    s += buf;
  }
  for (unsigned r = 0; r<n_rules; r++) {
    int coded_rule = 0;
    condition_t *rule = rules[r];
    for (signed c = n_conditions-1; c>=0; c--) {
      coded_rule<<=2;
      coded_rule |= rule[c];
    }
    sprintf(buf, "%u|%u|", coded_rule, actions[r]);
    s += buf;
  }

  return s;
}

int DecisionTable::InsertCondition(unsigned at, condition_id_t id)
{
  if (at>n_conditions) {
    LastError = OutOfRange;
    return -1;
  }
  n_conditions++;
  condition_ids = (condition_id_t*)realloc(condition_ids, n_conditions*sizeof(condition_id_t));
  for (signed c = n_conditions-1; c > at; c--) 
    condition_ids[c] = condition_ids[c-1];
  condition_ids[at] = id;    
  if (n_rules == 0) return at;
  for (unsigned r = 0; r<n_rules; r++) {
    condition_t *rule = rules[r] = (condition_t*)realloc(rules[r], 
      n_conditions*sizeof(condition_t));
    for (signed c = n_conditions-1; c > at; c--)
      rule[c] = rule[c-1];
    rule[at] = any;
  }
  LastError = Ok;
  return at;
}

int DecisionTable::InsertRule(unsigned at, int action)
{
  if (at>n_rules) {
    LastError = OutOfRange;
    return -1;
  }
  n_rules++;
  rules = (condition_t**)realloc(rules, n_rules*sizeof(condition_t*));
  actions = (action_t*)realloc(actions, n_rules*sizeof(action_t));
  for (signed r = n_rules-1; r > at; r--) {
    rules[r] = rules[r-1];
    actions[r] = actions[r-1];
  }
  condition_t *rule = rules[at] = new condition_t[n_conditions];
  for (unsigned c = 0; c<n_conditions; c++)
    rule[c] = any;
  actions[at] = action;
  LastError = Ok;
  return at;
}

void DecisionTable::SwapConditions(unsigned c1, unsigned c2)
{
  if (c1>=n_conditions || c2>=n_conditions) {
    LastError = OutOfRange;
    return;
  }

  condition_id_t id = condition_ids[c1];
  condition_ids[c1] = condition_ids[c2];
  condition_ids[c2] = id;

  for (unsigned r = 0; r<n_rules; r++) {
    condition_t *rule = rules[r];
    condition_t cond = rule[c1];
    rule[c1] = rule[c2];
    rule[c2] = cond;
  }
  LastError = Ok;
}

void DecisionTable::SwapRules(unsigned r1, unsigned r2)
{
  if (r1>=n_rules || r2>=n_rules) {
    LastError = OutOfRange;
    return;
  }
  action_t action = actions[r1];
  actions[r1] = actions[r2];
  actions[r2] = action;

  condition_t *rule = rules[r1];
  rules[r1] = rules[r2];
  rules[r2] = rule;

  LastError = Ok;
}

void DecisionTable::DeleteCondition(unsigned at)
{
  if (at>=n_conditions) {
    LastError = OutOfRange;
    return;
  }
  n_conditions--;
  for (unsigned c = at; c<n_conditions; c++)
    condition_ids[c] = condition_ids[c+1];
  for (unsigned r = 0; r<n_rules; r++) {
    condition_t *rule = rules[r];
    for (unsigned c = at; c<n_conditions; c++)
      rule[c] = rule[c+1];
  }
  LastError = Ok;
}

void DecisionTable::DeleteRule(unsigned at)
{
  if (at>=n_rules) {
    LastError = OutOfRange;
    return;
  }
  n_rules--;
  delete [] rules[at];
  for (unsigned r = at; r < n_rules; r++) {
    rules[r] = rules[r+1];
    actions[r] = actions[r+1];
  }
  LastError = Ok;
}

void DecisionTable::Allocate(unsigned init_rules, unsigned init_conditions)
{
  n_conditions = init_conditions;
  n_rules = init_rules;
  _conditions = new condition_t[n_conditions];
  condition_ids = new condition_id_t[n_conditions];
  actions = new action_t[n_rules];
  rules = new condition_t *[n_rules];
  for (unsigned r = 0; r<n_rules; r++)
    rules[r] = new condition_t[n_conditions];
  LastError = Ok;
}

DecisionTable::action_t DecisionTable::Execute(get_condition_t input_callback, void* context)
{
  LastError = Ok;
  for (unsigned c = 0; c<n_conditions; c++) 
    _conditions[c] = input_callback(condition_ids[c], context)? tt : ff;
  for (unsigned r = 0; r<n_rules; r++) {
    const condition_t *rule = rules[r];
    bool status = true;
    for (unsigned c = 0; c<n_conditions; c++) 
      switch(rule[c]) {
        case tt: status = status && (_conditions[c]==tt); break;
        case ff: status = status && (_conditions[c]==ff); break;
      }
    if (status)
      return actions[r];
  }
  return noaction;
}

DecisionTable::action_t DecisionTable::GetAction(unsigned rule)
{
  LastError = OutOfRange;
  if (rule>=n_rules) 
    return any;
  LastError = Ok;
  return actions[rule];
}

DecisionTable::condition_id_t DecisionTable::GetConditionId(unsigned condition)
{
  LastError = OutOfRange;
  if (condition>=n_conditions) 
    return nocondition_id;
  LastError = Ok;
  return condition_ids[condition];
}

DecisionTable::condition_t DecisionTable::GetCell(unsigned rule, unsigned condition)
{
  LastError = OutOfRange;
  if (rule>=n_rules) return any;
  if (condition>=n_conditions) 
    return any;
  LastError = Ok;
  return rules[rule][condition];
}

void DecisionTable::SetCell(unsigned rule, unsigned condition, condition_t content)
{
  LastError = OutOfRange;
  if (rule>=n_rules) 
    return;
  if (condition>=n_conditions) 
    return;
  LastError = Ok;
  rules[rule][condition] = content;
}

unsigned DecisionTable::GetRulesCount()
{
  return n_rules;
}

unsigned DecisionTable::GetConditionsCount()
{
  return n_conditions;
}

void DecisionTable::Expand()
{
  bool loop = true;
  if (n_rules == 0 || n_conditions == 0) return;
  while (loop) {
    loop = false;
    for (unsigned r = 0; r < n_rules; r++)
    {
      condition_t *rule1 = rules[r];
      for (unsigned c = 0; c < n_conditions; c++) 
      {
        if (rule1[c]==any) {
          loop = true;
          InsertRule(r+1, actions[r]);
          condition_t *rule2 = rules[r+1];
          for (unsigned c2 = 0; c2 < n_conditions; c2++) 
            rule2[c2] = rule1[c2];
          rule1[c] = ff;
          rule2[c] = tt;
        }
      }
    }
  }
}

void DecisionTable::Sort()
{
  if (n_rules == 0 || n_conditions == 0) return;
}

void DecisionTable::Dump(FILE* out) 
{
  fprintf(out, "RUL: ");
  for (unsigned r = 0; r<GetRulesCount(); r++) 
    fprintf(out, " %d", r);
  fprintf(out, "\n");
  for (unsigned c = 0; c<GetConditionsCount(); c++) {
    fprintf(out, "C%02d: ", GetConditionId(c));
    for (unsigned r = 0; r<GetRulesCount(); r++) {
      switch(GetCell(r, c)) {
        case DecisionTable::any: fprintf(out, " -"); break;
        case DecisionTable::tt: fprintf(out, " T"); break;
        case DecisionTable::ff: fprintf(out, " F"); break;
      }
    }
    fprintf(out, "\n");
  }
  fprintf(out, "ACT: ");
  for (unsigned r = 0; r<GetRulesCount(); r++) 
    fprintf(out, " %d", GetAction(r));
  fprintf(out, "\n");
}
