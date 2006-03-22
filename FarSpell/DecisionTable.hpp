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

#pragma once

#include "FARPlus/FARString.h"


class DecisionTable
{
  public:
    class Action {
      friend class DecisionTable;
      public:
        void *client_context;
      protected:
        int uid;
        virtual ~Action();
        virtual void OnSave() = 0;
        virtual void Execute() = 0;
    };
    class Condition {
      friend class DecisionTable;
      public:
        void *client_context;
      protected:
        int uid;
        virtual void OnSave() = 0;
        virtual bool Execute() = 0;
        virtual ~Condition();
    };
    typedef Condition *condition_inst_t;
    typedef Action *action_inst_t;
    class Context {
      friend class DecisionTable;
    protected:
      virtual condition_inst_t GetCondition(int uid) = 0;
      virtual condition_inst_t CreateCondition() = 0;
      virtual action_inst_t GetAction(int uid) = 0;
      virtual action_inst_t CreateAction() = 0;
    };
    typedef enum { any, ff, tt } condition_t;
  protected:
    Context *context;
    condition_t **rules;
    action_inst_t *action_insts;
    condition_inst_t *condition_insts;
    unsigned n_conditions;
    unsigned n_rules;
    condition_t *_condition_cache;
    void Allocate(unsigned init_rules, unsigned init_conditions);
  public:
    DecisionTable();
    //DecisionTable(DecisionTable &rhs);
    ~DecisionTable();
    void SetContext(Context *);
    action_inst_t Execute();
    void FromString(const FarString &source);
    FarString AsString();
    void Dump(FILE* out);
    condition_inst_t InsertCondition(unsigned at);
    void SwapConditions(unsigned c1, unsigned c2);
    void DeleteCondition(unsigned at);
    action_inst_t InsertRule(unsigned at);
    action_inst_t InsertRule(unsigned at, action_inst_t action);
    void SwapRules(unsigned r1, unsigned r2);
    void DeleteRule(unsigned at);
    action_inst_t GetActionInst(unsigned n_rule);
    condition_t GetCell(unsigned rule, unsigned n_condition);
    condition_inst_t GetConditionInst(unsigned n_condition);
    int GetConditionNumber(condition_inst_t inst);
    void SetCell(unsigned n_rule, unsigned n_condition, condition_t content);
    unsigned GetRulesCount();
    unsigned GetConditionsCount();
    enum {
      Ok,
      OutOfRange,
      InvalidSource
    } LastError;
  public: // algorithms
    void Expand();
    void Sort();
};
