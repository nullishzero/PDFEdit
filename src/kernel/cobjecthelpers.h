/*
 * PDFedit - free program for PDF document manipulation.
 * Copyright (C) 2006-2009  PDFedit team: Michal Hocko,
 *                                        Jozef Misutka,
 *                                        Martin Petricek
 *                   Former team members: Miroslav Jahoda
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program (in doc/LICENSE.GPL); if not, write to the 
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
 * MA  02111-1307  USA
 *
 * Project is hosted on http://sourceforge.net/projects/pdfedit
 */
// vim:tabstop=4:shiftwidth=4:noexpandtab:textwidth=80

#ifndef _COBJECTHELPERS_H_
#define _COBJECTHELPERS_H_

#include "kernel/cobject.h"

//=====================================================================================
namespace pdfobjects {
//=====================================================================================
namespace utils {
//=====================================================================================

/** Gets type of the dictionary.
 * @param dict Dictionary wrapped in smart pointer.
 *
 * Tries to get Type field from dictionary and returns its string value. If not
 * present, returns an empty string.
 *
 * @return string name of the dictionary type or empty string if not able to
 * find out.
 */
std::string getDictType(boost::shared_ptr<CDict> dict);
        
/** Helper method for getting int property value from dictionary.
 * @param name Name of the property in the dictionary.
 * @param dict Dictionary where to search.
 *
 * Gets property according name. Checks property type and if it is realy
 * pInt, gets its int value which is returned.
 *
 * @throw ElementNotFoundException if property is not found.
 * @throw ElementBadTypeException if property is found but doesn't contain 
 * integer value.
 * @return int value of the property.
 */
int getIntFromDict(std::string name, boost::shared_ptr<CDict> dict);

/** Helper method for getting reference property value from dictionary.
 * @param name Name of the property in the dictionary.
 * @param dict Dictionary where to search.
 *
 * Gets property according name. Checks property type and if it is realy
 * pRef, gets its IndiRef value which is returned.
 *
 * @throw ElementNotFoundException if property is not found.
 * @throw ElementBadTypeException if property is found but doesn't contain 
 * reference value.
 * @return IndiRef value of the property.
 */
IndiRef getRefFromDict(std::string name, boost::shared_ptr<CDict> dict);

/** Helper method for getting string property value from dictionary.
 * @param name Name of the property in the dictionary.
 * @param dict Dictionary where to search.
 *
 * Gets property according name. Checks property type and if it is realy
 * pString, gets its string value which is returned.
 *
 * @throw ElementNotFoundException if property is not found.
 * @throw ElementBadTypeException if property is found but doesn't contain 
 * string value.
 * @return std::string value of the property.
 */
std::string getStringFromDict(std::string name, boost::shared_ptr<CDict> dict);

/** Helper method for getting name property value from dictionary.
 * @param name Name of the property in the dictionary.
 * @param dict Dictionary where to search.
 *
 * Gets property according name. Checks property type and if it is realy
 * pName, gets its string value which is returned.
 *
 * @throw ElementNotFoundException if property is not found.
 * @throw ElementBadTypeException if property is found but doesn't contain 
 * string value.
 * @return std::string value of the property.
 */
std::string getNameFromDict(std::string name, boost::shared_ptr<CDict> dict);

/** Gets dictionary from reference property.
 * @param refProp Reference property (must be pRef typed).
 *
 * Gets reference value from property and dereferences indirect object from it.
 * Uses refProp's pdf for dereference.
 * Finaly casts (if indirect is dictionary) to CDict and returns.
 *
 * @throw ElementBadTypeException if refProp is not CRef instance or indirect
 * object is not CDict instance.
 * @return CDict instance wrapper by shared_ptr smart pointer.
 */
boost::shared_ptr<CDict> getDictFromRef(boost::shared_ptr<IProperty> refProp)DEPRECATED;

/** Gets dictionary from reference and pdf instance.
 * @param pdf Pdf istance.
 * @param ref Indirect reference.
 *
 * Uses CPdf::getIndirectProperty to get dereferenced object from given
 * reference on given pdf instance. Checks object type and if it is CDict,
 * returns it, otherwise throws an axception.
 *
 * @throw ElementBadTypeException if indirect object is not CDict instance.
 * @return CDict instance wrapper by shared_ptr smart pointer.
 */
boost::shared_ptr<CDict> getDictFromRef(CPdf & pdf, IndiRef ref)DEPRECATED;

/** Helper method for property printing.
 * @param ip Property to print.
 * @param out Output stream, where to print (implicit value is std::cout).
 *
 * Gets string representation of given property and dumps it to standard output.
 * Checks whether this functionality is duplicated.
 */
void printProperty(boost::shared_ptr<IProperty> ip, std::ostream &out);

//=========================================================

/** Equality functor for properties.
 * 
 * Class with function operator to compare two properties. Class can be used as
 * normal function.
 */
class PropertyEquals
{
public:
	/** Equality function operator.
	 * @param val1 Value to compare (with T type wrapped by smart poiter).
	 * @param val2 Value to compare (with T type wrapped by smart poiter).
	 *
	 * Two values are equal iff they:
	 * <ul>
	 * <li> have same types (getType returns same value)</li>
	 * <li><ul> have same values
	 * 	<li> if value is simple - direct values are same (getValue is used)</li>
	 * 	<li> if value is complex - same number of elements and elements with same
	 * 	identifier are equal</li>
	 * </ul></li>
	 * </ul>
	 *
	 * <b>Supported types</b><br>
	 * <ul>
			<li> pNull </li>
			<li> pBool </li>
			<li> pInt </li>
			<li> pReal </li>
			<li> pString </li>
			<li> pName </li>
			<li> pRef </li>
	 * </ul>
	 * @throw NotImplementedException if value type is not supported (see supported
	 * types table).
	 *
	 * @return true if values are equal, false otherwise.
	 */
	bool operator()(const boost::shared_ptr<IProperty> & val1, const boost::shared_ptr<IProperty> & val2) const;
};

/** Collector of all Id complying given condition.
 *
 * Full state class which builds internal container (which may be returned by
 * getContainer method) from entries given to functional operator and which
 * matches according given comparator. 
 * <br>
 * Container is type of storage where elements of IdType are stored. Comparator
 * has to implement functional operator with two IProperty types wrapped by
 * shared_ptr smart pointers. Comparator can throw only NotImplementedException
 * to say that it is unable to compare such properties. PropertyEquals functor
 * is used as default type for Comparator.
 * <br>
 * Container instance given, as reference, to the constructor is not encapsulated
 * by class and can expose its state by getContainer method. Class is therefore
 * not responsible for deallocation of its instance. Content of the container
 * can be cleared by reset method (each time new collection should be done, it
 * should be called).
 */
template <typename Container, typename IdType, typename Comparator=PropertyEquals> 
class IdCollector
{
	Container & container;
	const boost::shared_ptr<IProperty> & searched;
	const Comparator comp;
public:
	/** Constructor with initialization.
	 * @param cont Container where to store ids.
	 * @param search Property to be searched.
	 *
	 */
	IdCollector(Container & cont, const boost::shared_ptr<IProperty> & search)
		:container(cont), searched(search), comp(Comparator())
		{}

	/** Filters ids of properties which equals searched.
	 * @param entry Entry which contains identificator, property pair.
	 *
	 * If entry's property equals according Comparator with searched one, adds 
	 * id to the container. 
	 * <br>
	 * Operator can hadle situation when Comparator throws
	 * NotImplementedException and ignores such element.
	 */
	void operator()(std::pair<IdType, boost::shared_ptr<IProperty> > entry)
	{
		try
		{
			// uses propertyEquals method to compare properties
			// it may throw, if element is not supported by operator, we will
			// simply ignore such values
			if(comp(searched, entry.second))
			{
				utilsPrintDbg(debug::DBG_DBG, "Element matches at id="<<entry.first);
				container.push_back(entry.first);
			}
		}catch(NotImplementedException &)
		{
			// type of element is not supported
			utilsPrintDbg(debug::DBG_WARN, "Element comparing not supported for typ="<<entry.second->getType()<<". Ignoring element.");
		}
	}

	/**
	 * Returns container.
	 *
	 * @return Reference to container.
	 */
	Container & getContainer()
	{
		return container;
	}
	
	/** Clears container.
	 */
	void reset()
	{
		container.clear();
	}
};

/** Gets all identificators of property in parent complex type.
 * @param parent Complex value where to search.
 * @param child Property to search.
 * @param container Container, where to place all identificators (array
 * indexes).
 *
 * Uses CObjectComplex::forEach method with IdCollector functor to collect all
 * ids to given container.
 * <br>
 * Complex template parameter stands for type of CObjectComplex where to search.
 * This type has to provide typedef for propertyId and getAllPropertyNames,
 * getProperty methods.
 * <br>
 * Container template parameter stands for type of storage where to place found
 * indexes. This has to provide clear and push_back methods. Given container is
 * cleared at start.
 *
 */
template<typename Complex, typename Container > void 
getPropertyId(const boost::shared_ptr<Complex> & parent, const boost::shared_ptr<IProperty> &child, Container & container) throw()
{
	utilsPrintDbg(debug::DBG_DBG, "");

	// collects all properties identificators which are same as child
	// uses forEach with IdCollector functor
	typedef IdCollector<Container, typename Complex::PropertyId> IdCollectorType;
	IdCollectorType collector(container, child);
	parent->forEach(collector);
	
}


/** Creates property with correct pdf Rectangle representation.
 * @param rect Rectangle to convert.
 *
 * Created property is an array with four elements (see PDF specification
 * page 134 for more information).
 *
 * @return Property (CArray) wrapped by smart pointer.
 */
boost::shared_ptr<IProperty> getIPropertyFromRectangle(const libs::Rectangle & rect);

/** Gets rectangle value from given property.
 * @param prop Array property with exactly 4 elements.
 * @param rect Rectangle which will be filled with values stored in given 
 * property.
 *
 * Array property is transformed into the given rect object.
 * @throw CObjBadValue if given property has not valid rectangle representation.
 */
void getRectangleFromProperty(const boost::shared_ptr<IProperty> prop, libs::Rectangle & rect);


/** Creates property with correct pdf time representation.
 * @param time Time to convert.
 * 
 * Created property is an string with ASN.1 defined in ISO/IEC 8824 format 
 * used in pdf (see PDF specification page 133 for more information).
 */
boost::shared_ptr<IProperty> getIPropertyFromDate(const tm * time);


/**
 * Get all children of a tree like structre of pdf objects with, "Prev", Next", "First"
 * dictionary entries. This function can be used e.g. for getting all outlines.
 *
 * @param topdict Top level dictionary (it won't be included in the cont).
 * @param cont Output container of all children.
 */
template<typename Container>
void
getAllChildrenOfPdfObject (boost::shared_ptr<CDict> topdict, Container& cont)
{

	assert (topdict);

	//
	// Try to find any descandands
	//
	boost::shared_ptr<CDict> dict; 
	try {

		dict = topdict->getProperty<CDict>("First");

	}catch (ElementNotFoundException&) // No child
		{ return; }

	//
	// Children found
	// 
	while (true)
	{
		assert (dict);

		// Add object to container
		cont.push_back (dict);
		// Add all its children
		getAllChildrenOfPdfObject (dict, cont);
		
		try {
			dict = dict->getProperty<CDict>("Next");
		}catch (ElementNotFoundException&)
		{
			return;
		}
	}
}

/** Checks and replaces mapping in given dictionary.
 * @param annotDict Dictionary to check.
 * @param fieldName Field name to search.
 * @param fieldValue New value for this fieldName.
 *
 * Sets new value of given fieldName field and returns true if this property was
 * replaced, false if it was added.
 *
 * @return true if given fieldName's value has been overwirten, false otherwise.
 */
bool checkAndReplace(boost::shared_ptr<CDict> annotDict, std::string fieldName, IProperty & fieldValue);


/**
 * Get xref from an object.
 */
::XRef* getXRef (boost::shared_ptr<IProperty> ip);

//=====================================================================================
}// end of utils namespace
//=====================================================================================
}// end of pdfobjects namespace
//=====================================================================================

#endif

