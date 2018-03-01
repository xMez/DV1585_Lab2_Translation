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
		if(tExit)
                	cout << "True:  " << tExit->name << endl;
		if(fExit)
                	cout << "False: " << fExit->name << endl;
        }
};
int BBlock::nCounter = 0;


/******************** Expressions ********************/
class Expression
{
private:
	static int nCounter;
public:
        string name;

        Expression() : name("")
        {
        }
        virtual string makeNames() 
        {
		return name != "" ? name : "_t" + to_string(nCounter++);
          // Lecture 8 / slide 11.
          // Virtual (but not pure) to allow overriding in the leaves.
        }
        virtual string convert(BBlock*) = 0; // Lecture 8 / slide 12.

	virtual void dump(int depth) = 0;
};
int Expression::nCounter = 0;

class Variable : public Expression 
{
public:
	string name;

	Variable(string name) :
		name(name)
	{
	}

	string convert(BBlock *out)
	{
		return name;
	}

	void dump(int depth)
	{
		for(auto i=0; i<depth; i++)
			cout << "  ";
		cout << name << endl;
	}
};

class Constant : public Expression
{ 
public:
	string num;

	Constant(int num) :
		num(to_string(num))
	{
	}

	string convert(BBlock *out)
	{
		return num;
	}

	void dump(int depth)
	{
		for(auto i=0; i<depth; i++)
			cout << "  ";
		cout << num << endl;
	}
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
		string var = makeNames();
		out->instructions.push_back(ThreeAd(var, '+', lhs->convert(out), rhs->convert(out)));
		return var;
        }

	void dump(int depth)
	{
		for(auto i=0; i<depth; i++)
			cout << "  ";
		lhs->dump(depth);
		cout << "+" << endl;
		rhs->dump(depth);
	}

};

class Mult : public Expression
{
public:
	Expression *lhs, *rhs;

	Mult(Expression* lhs, Expression* rhs) :
		lhs(lhs), rhs(rhs)
	{
	}

	string convert(BBlock* out)
	{
		string var = makeNames();
		out->instructions.push_back(ThreeAd(var, '*', lhs->convert(out), rhs->convert(out)));
		return var;
	}

	void dump(int depth)
	{
		for(auto i=0; i<depth; i++)
			cout << "  ";
		lhs->dump(depth);
		cout << "*" << endl;
		rhs->dump(depth);
	}
};

class Equality : public Expression 
{
public:
	Expression *lhs, *rhs;

	Equality(Expression* lhs, Expression *rhs) :
		lhs(lhs), rhs(rhs)
	{
	}

	string convert(BBlock *out)
	{
		string var = makeNames();
		out->instructions.push_back(ThreeAd(var, '=', lhs->convert(out), rhs->convert(out)));
		return var;
	}

	void dump(int depth)
	{
		for(auto i=0; i<depth; i++)
			cout << "  ";
		cout << "==" << endl;
		lhs->dump(depth);
		rhs->dump(depth);
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

	virtual void dump(int depth=1) = 0;
};

class If : public Statement
{
public:
	Expression *cond;
	Statement *lhs, *rhs;

	If(Expression *cond, Statement *lhs, Statement *rhs) :
		cond(cond), lhs(lhs), rhs(rhs)
	{
	}

	BBlock* convert(BBlock *out)
	{
		cond->convert(out);

		BBlock* retBlock = new BBlock();

		BBlock* trueBlock = new BBlock();
		out->tExit = trueBlock;
		trueBlock = lhs->convert(trueBlock);
		trueBlock->tExit = retBlock;

		BBlock* falseBlock = new BBlock();
		out->fExit = falseBlock;
		falseBlock = rhs->convert(falseBlock);
		falseBlock->tExit = retBlock;

		return retBlock;
	}

	void dump(int depth)
	{
		for(auto i=0; i<depth; i++)
			cout << "  ";
		cout << "Statement(I)" << endl;
		cond->dump(depth);
		lhs->dump(depth+1);
		rhs->dump(depth+1);
	}
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
		string var = rhs->convert(out);
		out->instructions.push_back(ThreeAd(lhs->convert(out), 'c', var, var));
		return out;
        }

	void dump(int depth)
	{
		for(auto i=0; i<depth; i++)
			cout << "  ";
		cout << "StretBlockment(A)" << endl;
		lhs->dump(depth+1);
	      	rhs->dump(depth);
	}
};

class Seq : public Statement
{
public:
	list<Statement*> seqList;

	Seq(list<Statement*> seqList) :
		seqList(seqList)
	{
	}

	BBlock* convert(BBlock *out)
	{
		for(auto i : seqList)
			out = i->convert(out);
		return out;
	}

	void dump(int depth=1)
	{
		cout << "Statement(S)" << endl;
		for(auto i : seqList)
			i->dump(depth);
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

Statement *test22 = new Seq({
		new Assignment(
				"x",
				new Add(
					new Variable("x"),
					new Constant(1)
				       )
			      ),
		new Assignment(
				"y",
				new Add(
					new Variable("x"),
					new Constant(1)
				       )
			      ),
		new If(
				new Equality(
					new Variable("x"),
					new Constant(0)
					),
				new If(
					new Equality(
						new Variable("y"),
						new Constant(0)
						),
					new Assignment(
						"x",
						new Constant(1)
						),
					new Assignment(
						"y",
						new Constant(2)
						)
				      ),
				new Assignment(
					"y",
					new Constant(3)
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

int main(int argc, char *argv[])
{
	BBlock* block = new BBlock();
	test22->convert(block);
	dumpCFG(block);
	test22->dump();
	return 0;
}
