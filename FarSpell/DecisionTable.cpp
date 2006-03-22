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

DecisionTable::Action::~Action()
{
}

DecisionTable::Condition::~Condition()
{
}

DecisionTable::~DecisionTable()
{
  delete[] _condition_cache;
  for (int a = n_rules-1; a>=0; a--)
    delete action_insts[a];
  delete[] action_insts;
  for (int c = n_conditions-1; c>=0; c--)
    delete condition_insts[c];
  delete[] condition_insts;
  for (unsigned r = 0; r<n_rules; r++)
    delete[] rules[r];
  delete[] rules;
}

DecisionTable::DecisionTable()
{
  context = NULL;
  rules = NULL;
  action_insts  = NULL;
  condition_insts = NULL;
  n_conditions = n_rules = 0;
  _condition_cache = NULL;
  LastError = Ok;
}

void DecisionTable::SetContext(Context *in_context)
{
  context = in_context;
}

void DecisionTable::FromString(const FarString &source)
{
  unsigned i, r, c;
  FarStringTokenizer tokenizer (source, '|');

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

  for (c = 0; c<n_conditions && tokenizer.HasNext(); c++) {
    condition_insts[c] = context->GetCondition(atoi(tokenizer.NextToken()));
  }
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
    action_insts[r] = context->GetAction(atoi(tokenizer.NextToken()));
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
    condition_insts[c]->OnSave();
    sprintf(buf, "%u|", condition_insts[c]->uid);
    s += buf;
  }
  for (unsigned r = 0; r<n_rules; r++) {
    int coded_rule = 0;
    condition_t *rule = rules[r];
    for (signed c = n_conditions-1; c>=0; c--) {
      coded_rule<<=2;
      coded_rule |= rule[c];
    }
    action_insts[r]->OnSave();
    sprintf(buf, "%u|%u|", coded_rule, action_insts[r]->uid);
    s += buf;
  }

  return s;
}

DecisionTable::condition_inst_t DecisionTable::InsertCondition(unsigned at)
{
  if (at>n_conditions) {
    LastError = OutOfRange;
    return NULL;
  }
  n_conditions++;
  condition_insts = (condition_inst_t*)realloc(condition_insts, n_conditions*sizeof(condition_inst_t));
  for (signed c = n_conditions-1; c > at; c--) 
    condition_insts[c] = condition_insts[c-1];
  condition_inst_t inst = context->CreateCondition();
  condition_insts[at] = inst;
  if (n_rules == 0) return inst;
  for (unsigned r = 0; r<n_rules; r++) {
    condition_t *rule = rules[r] = (condition_t*)realloc(rules[r], 
      n_conditions*sizeof(condition_t));
    for (signed c = n_conditions-1; c > at; c--)
      rule[c] = rule[c-1];
    rule[at] = any;
  }
  LastError = Ok;
  return inst;
}

DecisionTable::action_inst_t DecisionTable::InsertRule(unsigned at, action_inst_t in_action)
{
  if (at>n_rules) {
    LastError = OutOfRange;
    return NULL;
  }
  n_rules++;
  rules = (condition_t**)realloc(rules, n_rules*sizeof(condition_t*));
  action_insts = (action_inst_t*)realloc(action_insts, n_rules*sizeof(action_inst_t));
  for (signed r = n_rules-1; r > at; r--) {
    rules[r] = rules[r-1];
    action_insts[r] = action_insts[r-1];
  }
  condition_t *rule = rules[at] = new condition_t[n_conditions];
  for (unsigned c = 0; c<n_conditions; c++)
    rule[c] = any;
  action_inst_t action;
  if (in_action)
    action = context->GetAction(in_action->uid);
  else
    action = context->CreateAction();
  action_insts[at] = action;
  LastError = Ok;
  return action;
}

DecisionTable::action_inst_t DecisionTable::InsertRule(unsigned at)
{
  return InsertRule(at, NULL);
}

void DecisionTable::SwapConditions(unsigned c1, unsigned c2)
{
  if (c1>=n_conditions || c2>=n_conditions) {
    LastError = OutOfRange;
    return;
  }

  condition_inst_t id = condition_insts[c1];
  condition_insts[c1] = condition_insts[c2];
  condition_insts[c2] = id;

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
  action_inst_t action = action_insts[r1];
  action_insts[r1] = action_insts[r2];
  action_insts[r2] = action;

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
  delete condition_insts[at];
  for (unsigned c = at; c<n_conditions; c++)
    condition_insts[c] = condition_insts[c+1];
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
  delete action_insts[at];
  for (unsigned r = at; r < n_rules; r++) {
    rules[r] = rules[r+1];
    action_insts[r] = action_insts[r+1];
  }
  LastError = Ok;
}

void DecisionTable::Allocate(unsigned init_rules, unsigned init_conditions)
{
  n_conditions = init_conditions;
  n_rules = init_rules;
  _condition_cache = new condition_t[n_conditions];
  condition_insts = new condition_inst_t[n_conditions];
  action_insts = new action_inst_t[n_rules];
  rules = new condition_t *[n_rules];
  for (unsigned r = 0; r<n_rules; r++)
    rules[r] = new condition_t[n_conditions];
  LastError = Ok;
}

DecisionTable::action_inst_t DecisionTable::Execute()
{
  LastError = Ok;
  for (unsigned c = 0; c<n_conditions; c++) 
    _condition_cache[c] = condition_insts[c]->Execute()? tt : ff;
  for (unsigned r = 0; r<n_rules; r++) {
    const condition_t *rule = rules[r];
    bool status = true;
    for (unsigned c = 0; c<n_conditions; c++) 
      switch(rule[c]) {
        case tt: status = status && (_condition_cache[c]==tt); break;
        case ff: status = status && (_condition_cache[c]==ff); break;
      }
    if (status)
      return action_insts[r];
  }
  return NULL;
}

DecisionTable::action_inst_t DecisionTable::GetActionInst(unsigned rule)
{
  LastError = OutOfRange;
  if (rule>=n_rules) 
    return NULL;
  LastError = Ok;
  return action_insts[rule];
}

DecisionTable::condition_inst_t DecisionTable::GetConditionInst(unsigned condition)
{
  LastError = OutOfRange;
  if (condition>=n_conditions) 
    return NULL;
  LastError = Ok;
  return condition_insts[condition];
}

int DecisionTable::GetConditionNumber(condition_inst_t inst)
{
  LastError = Ok;
  for (int c = 0; c<n_conditions; c++)
  {
    if (condition_insts[c] == inst)
      return c;
  }
  LastError = OutOfRange;
  return -1;
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
          InsertRule(r+1, action_insts[r]);
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
    fprintf(out, "C%02d: ", GetConditionInst(c)->uid);
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
    fprintf(out, " %d", GetActionInst(r)->uid);
  fprintf(out, "\n");
}
