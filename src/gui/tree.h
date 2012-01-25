/** \file tree.h contains declaration of item that should be used in tree-based models. They are bookmark handlings and analyze igem handling */
#ifndef __TREE__
#define __TREE__

#include <QString>
#include <QChar>
#include "typedefs.h"

/** \brief info about position of char in token */
/** this struct is used to determine where is the beginningof the matched string \n
*/
struct TokenInfo
{
	/** position of this char in the token */
	int _position;
	/** number of token from the start position were word was found */
	int _tokens;
	/** \brief operator = */
	TokenInfo operator-=(TokenInfo info)
	{
		_tokens-=info._tokens;
		_tokens = abs(_tokens);
		return *this;
	}
};
/** \brief base class for accepting char */
class Accept
{
protected:
	/// index of the state
	int _id;
	/// accepted chat
	QChar _ch;
	/// next state
	Accept * _next;
	/// where should state return
	Accept * _prev;
	/// information about state
	TokenInfo _info;
public:
	/** \brief constructor */
	Accept(QChar ch, Accept * prev, int id);
	
	/** \return the next class that could handle the input. If it return NULL, this state is end state */
	virtual Accept* accept(QChar ch, TokenInfo info);
	
	/** sets the next state */
	void setNext ( Accept * acc);
	
	/** get the next state */
	Accept * next()const ;
	
	/** sets where to return if the search fails */
	void setPrev ( Accept * acc);
	
	/**\brief returns where should this go in case of fail */
	Accept * prev()const;

	/** \brief check if this is end state (null is the next */
	bool isEnd()const;

	/** \brief check if this is begin state (previous is null */
	bool isBegin()const;
	
	/** \brief destructor */
	virtual ~Accept() ;
	
	/** \brief Checks if in this state it accepts the char */
	/** \retval true if it is able 
		\retval false otherwise
	*/
	virtual bool accepts(QChar ch);
	/// order of the state.States are linear, so this is its index
	int getId();
	/// information about char, which token it was anf wchich character in the token
	TokenInfo getPosition() 
	{
		return _info;
	}
};

/** \brief class for accepting sets */
/** this class will create linear list of char that is couls accetp. If it accepts any of it, it accepts */
class AcceptSet : public Accept
{
	/** every character that could be matched */
	std::vector< Accept *> acc;
public:
	/** constructor */
	AcceptSet(QString s, Accept * p, int id) ;
	
	/** reimplemented method */
	virtual Accept* accept(QChar ch, TokenInfo info);
	
	/** destructor **/
	virtual ~AcceptSet();
};

/** \brief class for matching every character */
/** accept will return true every time accept is called */
class AcceptDot : public Accept
{
public:
	/** \brief Constructor */
	AcceptDot(QChar ot, Accept * prev, int id) : Accept(ot, prev,id) {};
	/** \brief reimplemented accept */
	virtual Accept * accept(QChar c, TokenInfo info) 
	{
		_info = info;
		return next();
	}
};
/** \brief this class is not used */
class AcceptRange : public Accept
{
	int _beg, _end, _iter;
public:
	/** \brief Constructor */
	AcceptRange(QChar ot, int beg, int end, Accept * prev, int id);
	virtual Accept * accept(QChar c, TokenInfo info);
	virtual ~AcceptRange();
};
/** \brief main class responsible for searching */
class Tree
{
	std::vector<TokenInfo> infos;
	Accept * _root;
	Accept * _actual;
	QString _search;
	size_t _position;
	bool _regexp;
	bool _caseSensitive;
	bool _concateHyphen;
	bool _forward;
	/** creates i-th accepting state */
	void setAccept(QString pattern, int & i);
public:
	/** special chars that regexp accepts */
	enum Special
	{
		QUESTION= '?',
		STAR = '*',
		SLASH = '\\',
		LBRACKET = '[',
		RBRACKET =']',
		DOT = '.'
	};
/** \brief begin position in the token whre searched word was found */
	int _begin;
	/** \brief last position in the token whre searched word was found */
	int _end;
	/** \brief number of tokens between _begin and _end */
	int _tokens;

public:
	/** \brief output of the searching */
	enum TreeTokens
	{
		Next, 
		Found
	};
	/** clears all states that was set */
	void Clear();
	/** constructor */
	Tree();

	/** reverts pattern according to the rules */
	QString revertPattern(QString s);

	/** destructor */
	~Tree();

	/** sets the state according to the flags */
	bool setPattern(QString pattern);

public:
	/** sets token */
	void setText(QString text);
	/** perform search */
	TreeTokens search();
	/** set flags according which it should be searched */
	void setFlags( int flags ) ;
	/** not implemented yet. this should validate the pattern if there was regexpt */
	bool validateSearch( QString srch ) ;
	/** sets the string backwars because of backward  searching */
	QString revertNormal( QString text ) ;
	/** to revert correctly, special tokens must be taken care of */
	QString getNextSpecialToken( QString s, int& i ) ;
};
#endif  // __TREE__
