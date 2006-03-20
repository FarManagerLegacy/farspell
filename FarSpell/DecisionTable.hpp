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

class DecisionTable
{
  public:
    typedef enum { any, ff, tt } condition_t;
    typedef int condition_id_t;
    typedef int action_t;
    typedef bool (*get_condition_t)(condition_id_t id, void* context);
    static action_t noaction;
    static condition_id_t nocondition_id;
  protected:
    condition_t **rules;
    action_t *actions;
    condition_id_t *condition_ids;
    unsigned n_conditions;
    unsigned n_rules;
    condition_t *_conditions;
    void Allocate(unsigned init_rules, unsigned init_conditions);
  public:
    DecisionTable(const FarString &source);
    DecisionTable();
    //DecisionTable(DecisionTable &rhs);
    ~DecisionTable();
    action_t Execute(get_condition_t input_callback, void* context);
    FarString AsString();
    void Dump(FILE* out);
    int InsertCondition(unsigned at, condition_id_t id);
    void SwapConditions(unsigned c1, unsigned c2);
    void DeleteCondition(unsigned at);
    int InsertRule(unsigned at, action_t action);
    void SwapRules(unsigned r1, unsigned r2);
    void DeleteRule(unsigned at);
    action_t GetAction(unsigned rule);
    condition_t GetCell(unsigned rule, unsigned condition);
    condition_id_t GetConditionId(unsigned condition);
    void SetCell(unsigned rule, unsigned condition, condition_t content);
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
