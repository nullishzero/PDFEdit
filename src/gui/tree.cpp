#include "tree.h"

Accept::Accept(QChar ch, Accept * prev, int id) : _ch(ch), _next(NULL), _prev(prev),_id(id) { }

Accept* Accept::accept(QChar ch, TokenInfo info )
{
	if (ch != _ch)
		return _prev;
	_info = info;
	return _next;
}

bool Accept::accepts(QChar ch)
{
	TokenInfo i = {0,0};
	return accept(ch, i) == next();
}
void Accept::setNext ( Accept * acc) { _next = acc; }
Accept * Accept::next()const { return _next;}
void Accept::setPrev ( Accept * acc) { _prev = acc; }
Accept * Accept::prev()const { return _prev;}
bool Accept::isEnd()const { return _next == NULL; }
bool Accept::isBegin()const { return _prev == this; } //resp root, TODO vlastna podclass?Treba?
Accept::~Accept() {}

int Accept::getId()
{
	return _id;
}

AcceptSet::AcceptSet(QString s, Accept * p, int id) :Accept(' ', p,id)
{
	bool special = false;
	for (size_t i = 0; i < s.length(); i++)
	{
		if (s[i] == '\\')
		{
			special = true;
			continue;
		}
		if (special)
		{
			acc.push_back(new Accept(s[i], this,id));
			special = false;
			continue;
		}
		//osetrenie specialnych znakov
		if (s[i] == QChar('-'))
		{
			for (ushort c = s[i-1].unicode()+1; c<s[i+1].unicode()-1; c++)
				acc.push_back(new Accept(c,this,id)); //zmozina cez -
		}
	}
	_prev = p;
}
Accept* AcceptSet::accept(QChar ch, TokenInfo info)
{
	for(size_t i =0; i < acc.size(); i++)
	{
		if(acc[i]->accepts(ch))
		{
			_info = info;
			return next();
		}
	}
	return prev();
}
AcceptSet::~AcceptSet() {}
//class AcceptSpace : public Accept
//{
//	bool _accepted;
//public:
//	AcceptSpace(QChar ot, Accept * prev) : Accept(ot, prev), _accepted(false)	{	}
//	Accept * accept(QChar c) 
//	{
//		if ( c == ' ') //ak je to whitespace, TODO
//		{
//			_accepted = true;
//			if ( _next == NULL )
//				return NULL;
//			return this;
//		}
//		bool pom = _accepted;
//		_accepted = false; //do povodneho stavu
//		if (pom)
//			return _next->accept(c);
//		return _prev;
//	}
//};
// {0-19}

AcceptRange::AcceptRange(QChar ot, int beg, int end, Accept * prev, int id) : Accept(ot, prev,id)
{
	_beg = beg; _end = end; _iter = 0;
}

Accept * AcceptRange::accept(QChar c, TokenInfo info)  //zatial iba jedno pismeno
{
	if (c != _ch)
	{
		int i = _iter;
		_iter = 0;
		if ( i < _beg) //nedociahli sme na spodnu hranicu
			return prev();
		return next()->accept(c, info);
	}
	_iter ++;
	if (_iter < _end) 
		return this;
	_iter = 0;
	return _prev;
}
AcceptRange::~AcceptRange() {}

void Tree::Clear()
{
	while(_root!=NULL)
	{
		Accept * t = _root;
		_root = t->next();
		delete t; //TODO check
	}
}
Tree::Tree() : _regexp(false),_position (-1),_root(NULL),_caseSensitive(false),_concateHyphen(false),
_actual (NULL),_begin (0),_end (0),_tokens(-1)
{}

QString Tree::revertPattern(QString s)
{
	if (!_regexp)
		return revertNormal(s); //v tomto okamihu je string validny
	int i =0;
	QString res;
	while (i < s.size())
	{ //vsetky rozoznavane znaky-> .?*[]
		QString token = getNextSpecialToken(s,i);
		res.push_front(token);
	}
	return res;
}

Tree::~Tree() { Clear(); }

bool Tree::setPattern(QString pattern)
{
	if (!_forward)
		pattern = revertPattern(pattern);
	Clear();
	if (!validateSearch(pattern))
		return false;
	assert(!pattern.isEmpty()); //TODO validate
	pattern = pattern.trimmed();
	if (!_caseSensitive)
		pattern = pattern.toLower();
	Accept * prev = NULL;
	int i = 0;
	setAccept(pattern,i);
	_actual->setPrev(_actual); //back to root
	_root = _actual;
	std::vector<Accept *> _table;
	_table.clear();
	_table.push_back(_root);
	prev = _root;
	while (i < pattern.length())
	{	
		if ((pattern[i] == QChar(' '))||_concateHyphen && (pattern[i] == QChar('-')))
		{
			i++;
			continue;
		};
		setAccept(pattern,i);
		prev->setNext(_actual);
		prev = _actual;
		//vytvarame tabulku
		_table.push_back(_table.back());
		while (true)	
		{
			if ((_table.back())->accepts(pattern[i-1]))
			{
				_table.back() = (_table.back()->next());
				break;
			}//nekceptuje ale padame dolu
			if (_table.back() == _root)
				break;
			_table.back() = _table.back()->prev();
		}
		_actual->setPrev(_table.back());
		assert(_table.size() == i);
	}
	_actual = _root;
	//_table.clear();
	//_table.push_back(_root);
	//while (_table.back())
	//	_table.push_back(_table.back()->next()); // pre zaverecnu integritu
	//_table.pop_back();
	return true;
} //krajsie by to bolo asi odzadu ale co uz


void Tree::setAccept(QString pattern, int & i)
{
	if (!_regexp)
	{
		_actual = new Accept(pattern[i],_root,i);
		i++;
		return;
	}
	if (pattern[i] == QChar('*'))
	{
		_actual = new AcceptRange(pattern[i],0,~0,_root,i);
		i+=2; //zobrali sme aj dalsie				
	}
	else if (pattern[i] == QChar('\\'))
	{
		_actual = new Accept(pattern[i+1],_root,i);
		i +=2;
	}
	else if (pattern[i] == QChar('['))
	{
		int j = i;
		while ((j = pattern.indexOf("]", j)) != -1) {
			if (pattern[j-1] != '\\')
				break;
			++j;
		}
		QString res = pattern.mid(i+1,j-i-1);
		_actual = new AcceptSet(res,_root,i);
		i = j+1;
	}
	else if (pattern[i+1] == QChar('.'))
	{
		_actual = new AcceptDot(pattern[i],_root,i);
		i += 2;
	}
	//else if (pattern[i] == QChar(' '))
	//{
	//	if (_actual && _actual->getChar()!= ' ')
	//	{
	//		_actual = new AcceptSpace(pattern[i],_root);
	//	}
	//	i++; //spracovane
	//}
	else
	{ 
		_actual = new Accept(pattern[i],_root,i);
		i++;
	}
}
void Tree::setText(QString text)
{
	_tokens++;
	if (!_caseSensitive)
		text = text.toLower();
	if (!_forward)
		text = revertNormal(text);
	_search = text;
}
//ak je tj operator s whitespacom ->"test    test", "test \t test"
//search - ..test -> .test OK
//search - xx..test -> xx.text -> xx.(.)x

Tree::TreeTokens Tree::search()
{
	_position++; //pre pokracujuce veci
	//stejne toho nedostane vela a bude to brat po tokenoch
	for (; _position< _search.length();  _position++)
	{
		if (_concateHyphen && _search[_position]==QChar('-'))
			continue; //automaticky accept
		if (_actual->isBegin())
		{
			_tokens = 0;
			_begin = _position;
		}
		bool done = false;
		int oldId = _actual->getId();
		while (!done)	
		{
			if (_actual == _root || _actual->accepts(_search[_position]))
				done = true;
			TokenInfo info = {_position,_tokens};
			_actual = _actual->accept(_search[_position],info);
		}
		if (_actual==NULL) //posledne
		{
			_actual = _root;
			_end = _position;
			return Tree::Found;//kolkate pismeno to bolo. Operator budeme vediet z toho, co tam vrazame
		}
		if (oldId > _actual->getId())
		{
			int diff = oldId - _actual->getId()-1;
			Accept * t = _root;
			for ( int i =0; i<diff; i++)
				t = t->next();
			_begin = t->getPosition()._position;
			_tokens -= t->getPosition()._tokens;
		}
	}
	_position = -1;
	return Tree::Next;
}

void Tree::setFlags( int flags ) 
{
	_caseSensitive = flags & SearchCaseSensitive;
	_concateHyphen = flags & SearchConcate;
	_regexp = flags & SearchRegexp;
	_forward = flags & SearchForward;
}

bool Tree::validateSearch( QString srch ) 
{
	//if (!_regexp)
	//	return true;
	////pravidla pre regularny vyraz
	//bool previousWasSpecial = true;
	//for ( int i =0; i< srch.size(); i++)
	//{

	//}
	return true;
}

QString Tree::revertNormal( QString text ) 
{
	QString res;
	for ( int i =0; i < text.size(); i++)
		res.push_front(text[i]);
	return res;
}

QString Tree::getNextSpecialToken( QString s, int& i ) 
{
	switch (s[i].unicode())
	{
	case QUESTION:
	case STAR:
	case SLASH:
		{
			QString ret;
			ret = QString(s[i]) + s[i+1];
			i+=2;
			return ret;
		}
	case LBRACKET:
		{
			int index = s.indexOf(']',i);
			QString ret = s.mid(i,index - i+1); //vratane zavoriek
			i = index+1;
			return ret;
		}
	case RBRACKET:
		assert (false);
	default:
		i++;
		return QString(s[i-1]);
	}
}
