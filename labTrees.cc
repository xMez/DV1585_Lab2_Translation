/* DV1465 / DV1505 / DV1511 Lab-task example code.
   (C) Dr Andrew Moss, Erik Bergenholtz  2016, 2017, 2018
   This code is released into the public domain.

   You are free to use this code as a base for your second assignment after
   the lab sessions (it is not required that you do so). 

   2018: Took out the double-pointers.
*/
#include <list>
#include <set>
#include <initializer_list>
#include <string>
#include <iostream>

using namespace std;

/************* Three Address Instructions *************/
class ThreeAd
{
public:
        string name,lhs,rhs;
        char op;

        ThreeAd(string name, char op, string lhs, string rhs) :
                name(name), op(op), lhs(lhs), rhs(rhs)
        {
        }

        void dump()
        {
                cout << name << " <- ";
                cout << lhs << " " << op << " " << rhs << endl;
        }
};


/* Basic Blocks */
class BBlock
{
private:
        static int nCounter;
public:
        list<ThreeAd> instructions;
        BBlock *tExit, *fExit;
        string name;

        BBlock() :
                tExit(NULL), fExit(NULL), name("blk" + to_string(nCounter++))
        {
        }

        void dump()
        {
                cout << "BBlock @ " << this << endl;
                cout << name << endl;
                for(auto i : instructions)
                        i.dump();
                cout << "True:  " << tExit << endl;
                cout << "False: " << fExit << endl;
        }
};
int BBlock::nCounter = 0;


/******************** Expressions ********************/
class Expression
{
public:
        string name;

        Expression() : name("")
        {
        }
        virtual string makeNames() 
        {
          // Lecture 8 / slide 11.
          // Virtual (but not pure) to allow overriding in the leaves.
        }
        virtual string convert(BBlock*) = 0; // Lecture 8 / slide 12.
};


class Add : public Expression
{
public:
        Expression *lhs, *rhs;

        Add(Expression* lhs, Expression* rhs) :
                lhs(lhs), rhs(rhs)
        {
        }

        string convert(BBlock* out)
        {
                // Write three address instructions to output
        }

};


/******************** Statements ********************/
class Statement
{
public:
        string name;

        Statement()
        {
        }
        virtual BBlock* convert(BBlock *) = 0;
};


class Assignment : public Statement
{
public:
        Variable *lhs;
        Expression *rhs;

        Assignment(string lhs, Expression *rhs) :
                lhs(new Variable(lhs)), rhs(rhs)
        {
        }

        BBlock* convert(BBlock *out)
        {
                // Write three address instructions to output
        }
};


/* Test cases */
Statement *test = new Seq({
                          new Assignment(
                                  "x",
                                  new Add(
                                          new Variable("x"),
                                          new Constant(1)
                                  )
                          ),new If(
                                  new Equality(
                                          new Variable("x"),
                                          new Constant(10)
                                  ),new Assignment(
                                          "y",
                                          new Add(
                                                  new Variable("x"),
                                                  new Constant(1)
                                          )
                                  ), new Assignment(
                                          "y",
                                          new Mult(
                                                  new Variable("x"),
                                                  new Constant(2)
                                          )
                                  )
                          ), new Assignment(
                                  "x",
                                  new Add(
                                          new Variable("x"),
                                          new Constant(1)
                                  )
                          )
});


/*
 * Iterate over each basic block that can be reached from the entry point
 * exactly once, so that we can dump out the entire graph.
 * This is a concrete example of the graph-walk described in lecture 7.
 */
void dumpCFG(BBlock *start)
{
        set<BBlock *> done, todo;
        todo.insert(start);
        while(todo.size()>0)
        {
                // Pop an arbitrary element from todo set
                auto first = todo.begin();
                BBlock *next = *first;
                todo.erase(first);
                next->dump();
                done.insert(next);
                if(next->tExit!=NULL && done.find(next->tExit)==done.end())
                        todo.insert(next->tExit);
                if(next->fExit!=NULL && done.find(next->fExit)==done.end())
                        todo.insert(next->fExit);
        }
}
