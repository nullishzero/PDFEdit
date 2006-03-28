/*
 * =====================================================================================
 *        Filename:  ccontentstream.cc
 *         Created:  03/24/2006 10:33:34 PM CET
 *          Author:  jmisutka (), 
 * =====================================================================================
 */

// static
#include "static.h"
//
#include "cobject.h"
#include "ccontentstream.h"


//==========================================================
namespace pdfobjects {
//==========================================================

using namespace std;
using namespace boost;
using namespace debug;
using namespace utils;


//==========================================================
// Specialized classes representing operators
//==========================================================

namespace
{
	//
	// Known operators, extern is a must here, does NOT need to be
	// sorted
	//
	extern const char OPER__SLASH[] = "\\"; 
	extern const char OPER__APO[] = "'"; 
	extern const char OPER_B[] = "B"; 
	extern const char OPER_B_STAR[] = "B*"; 
	extern const char OPER_BDC[] = "BDC" ; 
	extern const char OPER_BI[] = "BI"; 
	extern const char OPER_BMC[] = "BMC";
	extern const char OPER_BX[] =  "BX";
	extern const char OPER_CS[] =  "CS";
	extern const char OPER_DP[] =  "DP";
	extern const char OPER_Do[] =  "Do";
	extern const char OPER_EI[] =  "EI";
	extern const char OPER_EMC[] =  "EMC";
	extern const char OPER_ET[] =  "ET";
	extern const char OPER_EX[] =  "EX";
	extern const char OPER_F[] =  "F";
	extern const char OPER_G[] =  "G";
	extern const char OPER_ID[] =  "ID";
	extern const char OPER_J[] =  "J";
	extern const char OPER_K[] =  "K";
	extern const char OPER_M[] =  "M";
	extern const char OPER_MP[] = "MP";
	extern const char OPER_Q[] =  "Q";
	extern const char OPER_RG[] = "RG";
	extern const char OPER_S[] = "S";
	extern const char OPER_SC[] = "SC";
	extern const char OPER_SCN[] = "SCN";
	extern const char OPER_T_STAR[] = "T*";
	extern const char OPER_TD[] = "TD";
	extern const char OPER_TJ[] = "TJ";
	extern const char OPER_TL[] = "TL";
	extern const char OPER_Tc[] = "Tc";
	extern const char OPER_Td[] = "Td";
	extern const char OPER_Tf[] = "Tf";
	extern const char OPER_Tj[] = "Tj";
	extern const char OPER_Tm[] = "Tm";
	extern const char OPER_Tr[] = "Tr";
	extern const char OPER_Ts[] = "Ts";
	extern const char OPER_Tw[] = "Tw";
	extern const char OPER_Tz[] = "Tz";
	extern const char OPER_W[] = "W";
	extern const char OPER_W_STAR[] = "W*";
	extern const char OPER_b[] = "b";
	extern const char OPER_b_STAR[] = "b*";
	extern const char OPER_c[] = "c";
	extern const char OPER_cm[] = "cm";
	extern const char OPER_cs[] = "cs";
	extern const char OPER_d[] = "d";
	extern const char OPER_d0[] = "d0";
	extern const char OPER_d1[] = "d1";
	extern const char OPER_f[] = "f";
	extern const char OPER_f_STAR[] = "f*";
	extern const char OPER_g[] = "g";
	extern const char OPER_gs[] = "gs";
	extern const char OPER_h[] = "h";
	extern const char OPER_i[] = "i";
	extern const char OPER_j[] = "j";
	extern const char OPER_k[] = "k";
	extern const char OPER_l[] = "l";
	extern const char OPER_m[] = "m";
	extern const char OPER_n[] = "n";
	extern const char OPER_q[] = "q";
	extern const char OPER_re[] = "re";
	extern const char OPER_rg[] = "rg";
	extern const char OPER_ri[] = "ri";
	extern const char OPER_s[] = "s";
	extern const char OPER_sc[] = "sc";
	extern const char OPER_scn[] = "scn";
	extern const char OPER_sh[] = "sh";
	extern const char OPER_v[] = "v";
	extern const char OPER_w[] = "w";
	extern const char OPER_y[] = "y";
		
	/**
	 *
	 */
	class KnownOperators
	{
		public:
			/**
			 * Structure representing operator and function for creating specified operator.
			 *
			 * It is represented in a structure to optimize searching in throught the text 
			 * representing operators.
			 * 
			 */
			typedef struct
			{
				const char* name;					// Operator
				shared_ptr<PdfOperator> (*func) 	// Creator function
						(PdfOperator::Operands& operands);
				
			} OperatorTab;


		private:
			static const OperatorTab tab[];
			
		public:

			/**
			 * Find the operator and create it.
			 *
			 * @param op String representation of the operator. It is used to find the operator and
			 * 			 sometimes to initialize it.
			 * @param operands Operand stack.
			 *
			 * @return Created pdf operator.
			 */
			static shared_ptr<PdfOperator>
			findOp (const string& op, PdfOperator::Operands& operands);

			//
			// Creators
			//
			static shared_ptr<PdfOperator> createOperator__SLASH (PdfOperator::Operands& operands)
				{typedef SimpleGenericOperator<mpl::vector1_c<PropertyType, pString>, OPER__SLASH> COper;
					return shared_ptr<PdfOperator> (new COper (operands)); };

			static shared_ptr<PdfOperator> createOperator__APO (PdfOperator::Operands& operands)
				{typedef SimpleGenericOperator<mpl::vector1_c<PropertyType, pString>, OPER__APO> COper;
				 return shared_ptr<PdfOperator> (new COper (operands)); };

			static shared_ptr<PdfOperator> createOperator_B (PdfOperator::Operands& operands)
				{typedef SimpleGenericOperator<mpl::vector1_c<PropertyType, pString>, OPER_B> COper;
				 return shared_ptr<PdfOperator> (new COper (operands)); };

			static shared_ptr<PdfOperator> createOperator_B_STAR (PdfOperator::Operands& operands)
				{typedef SimpleGenericOperator<mpl::vector1_c<PropertyType, pString>, OPER_B_STAR> COper;
				 return shared_ptr<PdfOperator> (new COper (operands)); };

				static shared_ptr<PdfOperator> createOperator_BDC (PdfOperator::Operands& operands)
				{typedef SimpleGenericOperator<mpl::vector1_c<PropertyType, pString>, OPER_BDC> COper;
				 return shared_ptr<PdfOperator> (new COper (operands)); };

				static shared_ptr<PdfOperator> createOperator_BI (PdfOperator::Operands& operands)
				{typedef SimpleGenericOperator<mpl::vector1_c<PropertyType, pString>, OPER_BI> COper;
				 return shared_ptr<PdfOperator> (new COper (operands)); };

				static shared_ptr<PdfOperator> createOperator_BMC (PdfOperator::Operands& operands)
				{typedef SimpleGenericOperator<mpl::vector1_c<PropertyType, pString>, OPER_BMC> COper;
				 return shared_ptr<PdfOperator> (new COper (operands)); };

				static shared_ptr<PdfOperator> createOperator_BX (PdfOperator::Operands& operands)
				{typedef SimpleGenericOperator<mpl::vector1_c<PropertyType, pString>, OPER_BX> COper;
				 return shared_ptr<PdfOperator> (new COper (operands)); };

				static shared_ptr<PdfOperator> createOperator_CS (PdfOperator::Operands& operands)
				{typedef SimpleGenericOperator<mpl::vector1_c<PropertyType, pString>, OPER_CS> COper;
				 return shared_ptr<PdfOperator> (new COper (operands)); };

				static shared_ptr<PdfOperator> createOperator_DP (PdfOperator::Operands& operands)
				{typedef SimpleGenericOperator<mpl::vector1_c<PropertyType, pString>, OPER_DP> COper;
				 return shared_ptr<PdfOperator> (new COper (operands)); };

				static shared_ptr<PdfOperator> createOperator_Do (PdfOperator::Operands& operands)
				{typedef SimpleGenericOperator<mpl::vector1_c<PropertyType, pString>, OPER_Do> COper;
				 return shared_ptr<PdfOperator> (new COper (operands)); };

				static shared_ptr<PdfOperator> createOperator_EI (PdfOperator::Operands& operands)
				{typedef SimpleGenericOperator<mpl::vector1_c<PropertyType, pString>, OPER_EI> COper;
				 return shared_ptr<PdfOperator> (new COper (operands)); };

				static shared_ptr<PdfOperator> createOperator_EMC (PdfOperator::Operands& operands)
				{typedef SimpleGenericOperator<mpl::vector1_c<PropertyType, pString>, OPER_EMC> COper;
				 return shared_ptr<PdfOperator> (new COper (operands)); };

				static shared_ptr<PdfOperator> createOperator_ET (PdfOperator::Operands& operands)
				{typedef SimpleGenericOperator<mpl::vector1_c<PropertyType, pString>, OPER_ET> COper;
				 return shared_ptr<PdfOperator> (new COper (operands)); };

				static shared_ptr<PdfOperator> createOperator_EX (PdfOperator::Operands& operands)
				{typedef SimpleGenericOperator<mpl::vector1_c<PropertyType, pString>, OPER_EX> COper;
				 return shared_ptr<PdfOperator> (new COper (operands)); };

				static shared_ptr<PdfOperator> createOperator_F (PdfOperator::Operands& operands)
				{typedef SimpleGenericOperator<mpl::vector1_c<PropertyType, pString>, OPER_F> COper;
				 return shared_ptr<PdfOperator> (new COper (operands)); };

				static shared_ptr<PdfOperator> createOperator_G (PdfOperator::Operands& operands)
				{typedef SimpleGenericOperator<mpl::vector1_c<PropertyType, pString>, OPER_G> COper;
				 return shared_ptr<PdfOperator> (new COper (operands)); };

				static shared_ptr<PdfOperator> createOperator_ID (PdfOperator::Operands& operands)
				{typedef SimpleGenericOperator<mpl::vector1_c<PropertyType, pString>, OPER_ID> COper;
				 return shared_ptr<PdfOperator> (new COper (operands)); };

				static shared_ptr<PdfOperator> createOperator_J (PdfOperator::Operands& operands)
				{typedef SimpleGenericOperator<mpl::vector1_c<PropertyType, pString>, OPER_J> COper;
				 return shared_ptr<PdfOperator> (new COper (operands)); };

				static shared_ptr<PdfOperator> createOperator_K (PdfOperator::Operands& operands)
				{typedef SimpleGenericOperator<mpl::vector1_c<PropertyType, pString>, OPER_K> COper;
				 return shared_ptr<PdfOperator> (new COper (operands)); };

				static shared_ptr<PdfOperator> createOperator_M (PdfOperator::Operands& operands)
				{typedef SimpleGenericOperator<mpl::vector1_c<PropertyType, pString>, OPER_M> COper;
				 return shared_ptr<PdfOperator> (new COper (operands)); };

				static shared_ptr<PdfOperator> createOperator_MP (PdfOperator::Operands& operands)
				{typedef SimpleGenericOperator<mpl::vector1_c<PropertyType, pString>, OPER_MP> COper;
				 return shared_ptr<PdfOperator> (new COper (operands)); };

				static shared_ptr<PdfOperator> createOperator_Q (PdfOperator::Operands& operands)
				{typedef SimpleGenericOperator<mpl::vector1_c<PropertyType, pString>, OPER_Q> COper;
				 return shared_ptr<PdfOperator> (new COper (operands)); };

				static shared_ptr<PdfOperator> createOperator_RG (PdfOperator::Operands& operands)
				{typedef SimpleGenericOperator<mpl::vector1_c<PropertyType, pString>, OPER_RG> COper;
				 return shared_ptr<PdfOperator> (new COper (operands)); };

				static shared_ptr<PdfOperator> createOperator_S (PdfOperator::Operands& operands)
				{typedef SimpleGenericOperator<mpl::vector1_c<PropertyType, pString>, OPER_S> COper;
				 return shared_ptr<PdfOperator> (new COper (operands)); };

				static shared_ptr<PdfOperator> createOperator_SC (PdfOperator::Operands& operands)
				{typedef SimpleGenericOperator<mpl::vector1_c<PropertyType, pString>, OPER_SC> COper;
				 return shared_ptr<PdfOperator> (new COper (operands)); };

				static shared_ptr<PdfOperator> createOperator_SCN (PdfOperator::Operands& operands)
				{typedef SimpleGenericOperator<mpl::vector1_c<PropertyType, pString>, OPER_SCN> COper;
				 return shared_ptr<PdfOperator> (new COper (operands)); };

				static shared_ptr<PdfOperator> createOperator_T_STAR (PdfOperator::Operands& operands)
				{typedef SimpleGenericOperator<mpl::vector1_c<PropertyType, pString>, OPER_T_STAR> COper;
				 return shared_ptr<PdfOperator> (new COper (operands)); };

				static shared_ptr<PdfOperator> createOperator_TD (PdfOperator::Operands& operands)
				{typedef SimpleGenericOperator<mpl::vector1_c<PropertyType, pString>, OPER_TD> COper;
				 return shared_ptr<PdfOperator> (new COper (operands)); };

				static shared_ptr<PdfOperator> createOperator_TJ (PdfOperator::Operands& operands)
				{typedef SimpleGenericOperator<mpl::vector1_c<PropertyType, pString>, OPER_TJ> COper;
				 return shared_ptr<PdfOperator> (new COper (operands)); };

				static shared_ptr<PdfOperator> createOperator_TL (PdfOperator::Operands& operands)
				{typedef SimpleGenericOperator<mpl::vector1_c<PropertyType, pString>, OPER_TL> COper;
				 return shared_ptr<PdfOperator> (new COper (operands)); };

				static shared_ptr<PdfOperator> createOperator_Tc (PdfOperator::Operands& operands)
				{typedef SimpleGenericOperator<mpl::vector1_c<PropertyType, pString>, OPER_Tc> COper;
				 return shared_ptr<PdfOperator> (new COper (operands)); };

				static shared_ptr<PdfOperator> createOperator_Td (PdfOperator::Operands& operands)
				{typedef SimpleGenericOperator<mpl::vector1_c<PropertyType, pString>, OPER_Td> COper;
				 return shared_ptr<PdfOperator> (new COper (operands)); };

				static shared_ptr<PdfOperator> createOperator_Tf (PdfOperator::Operands& operands)
				{typedef SimpleGenericOperator<mpl::vector1_c<PropertyType, pString>, OPER_Tf> COper;
				 return shared_ptr<PdfOperator> (new COper (operands)); };

				static shared_ptr<PdfOperator> createOperator_Tj (PdfOperator::Operands& operands)
				{typedef SimpleGenericOperator<mpl::vector1_c<PropertyType, pString>, OPER_Tj> COper;
				 return shared_ptr<PdfOperator> (new COper (operands)); };

				static shared_ptr<PdfOperator> createOperator_Tm (PdfOperator::Operands& operands)
				{typedef SimpleGenericOperator<mpl::vector1_c<PropertyType, pString>, OPER_Tm> COper;
				 return shared_ptr<PdfOperator> (new COper (operands)); };

				static shared_ptr<PdfOperator> createOperator_Tr (PdfOperator::Operands& operands)
				{typedef SimpleGenericOperator<mpl::vector1_c<PropertyType, pString>, OPER_Tr> COper;
				 return shared_ptr<PdfOperator> (new COper (operands)); };

				static shared_ptr<PdfOperator> createOperator_Ts (PdfOperator::Operands& operands)
				{typedef SimpleGenericOperator<mpl::vector1_c<PropertyType, pString>, OPER_Ts> COper;
				 return shared_ptr<PdfOperator> (new COper (operands)); };

				static shared_ptr<PdfOperator> createOperator_Tw (PdfOperator::Operands& operands)
				{typedef SimpleGenericOperator<mpl::vector1_c<PropertyType, pString>, OPER_Tw> COper;
				 return shared_ptr<PdfOperator> (new COper (operands)); };

				static shared_ptr<PdfOperator> createOperator_Tz (PdfOperator::Operands& operands)
				{typedef SimpleGenericOperator<mpl::vector1_c<PropertyType, pString>, OPER_Tz> COper;
				 return shared_ptr<PdfOperator> (new COper (operands)); };

				static shared_ptr<PdfOperator> createOperator_W (PdfOperator::Operands& operands)
				{typedef SimpleGenericOperator<mpl::vector1_c<PropertyType, pString>, OPER_W> COper;
				 return shared_ptr<PdfOperator> (new COper (operands)); };

				static shared_ptr<PdfOperator> createOperator_W_STAR (PdfOperator::Operands& operands)
				{typedef SimpleGenericOperator<mpl::vector1_c<PropertyType, pString>, OPER_W_STAR> COper;
				 return shared_ptr<PdfOperator> (new COper (operands)); };

				static shared_ptr<PdfOperator> createOperator_b (PdfOperator::Operands& operands)
				{typedef SimpleGenericOperator<mpl::vector1_c<PropertyType, pString>, OPER_b> COper;
				 return shared_ptr<PdfOperator> (new COper (operands)); };

				static shared_ptr<PdfOperator> createOperator_b_STAR (PdfOperator::Operands& operands)
				{typedef SimpleGenericOperator<mpl::vector1_c<PropertyType, pString>, OPER_b_STAR> COper;
				 return shared_ptr<PdfOperator> (new COper (operands)); };

				static shared_ptr<PdfOperator> createOperator_c (PdfOperator::Operands& operands)
				{typedef SimpleGenericOperator<mpl::vector1_c<PropertyType, pString>, OPER_c> COper;
				 return shared_ptr<PdfOperator> (new COper (operands)); };

				static shared_ptr<PdfOperator> createOperator_cm (PdfOperator::Operands& operands)
				{typedef SimpleGenericOperator<mpl::vector1_c<PropertyType, pString>, OPER_cm> COper;
				 return shared_ptr<PdfOperator> (new COper (operands)); };

				static shared_ptr<PdfOperator> createOperator_cs (PdfOperator::Operands& operands)
				{typedef SimpleGenericOperator<mpl::vector1_c<PropertyType, pString>, OPER_cs> COper;
				 return shared_ptr<PdfOperator> (new COper (operands)); };

				static shared_ptr<PdfOperator> createOperator_d (PdfOperator::Operands& operands)
				{typedef SimpleGenericOperator<mpl::vector1_c<PropertyType, pString>, OPER_d> COper;
				 return shared_ptr<PdfOperator> (new COper (operands)); };

				static shared_ptr<PdfOperator> createOperator_d0 (PdfOperator::Operands& operands)
				{typedef SimpleGenericOperator<mpl::vector1_c<PropertyType, pString>, OPER_d0> COper;
				 return shared_ptr<PdfOperator> (new COper (operands)); };

				static shared_ptr<PdfOperator> createOperator_d1 (PdfOperator::Operands& operands)
				{typedef SimpleGenericOperator<mpl::vector1_c<PropertyType, pString>, OPER_d1> COper;
				 return shared_ptr<PdfOperator> (new COper (operands)); };

				static shared_ptr<PdfOperator> createOperator_f (PdfOperator::Operands& operands)
				{typedef SimpleGenericOperator<mpl::vector1_c<PropertyType, pString>, OPER_f> COper;
				 return shared_ptr<PdfOperator> (new COper (operands)); };

				static shared_ptr<PdfOperator> createOperator_f_STAR (PdfOperator::Operands& operands)
				{typedef SimpleGenericOperator<mpl::vector1_c<PropertyType, pString>, OPER_f_STAR> COper;
				 return shared_ptr<PdfOperator> (new COper (operands)); };

				static shared_ptr<PdfOperator> createOperator_g (PdfOperator::Operands& operands)
				{typedef SimpleGenericOperator<mpl::vector1_c<PropertyType, pString>, OPER_g> COper;
				 return shared_ptr<PdfOperator> (new COper (operands)); };

				static shared_ptr<PdfOperator> createOperator_gs (PdfOperator::Operands& operands)
				{typedef SimpleGenericOperator<mpl::vector1_c<PropertyType, pString>, OPER_gs> COper;
				 return shared_ptr<PdfOperator> (new COper (operands)); };

				static shared_ptr<PdfOperator> createOperator_h (PdfOperator::Operands& operands)
				{typedef SimpleGenericOperator<mpl::vector1_c<PropertyType, pString>, OPER_h> COper;
				 return shared_ptr<PdfOperator> (new COper (operands)); };

				static shared_ptr<PdfOperator> createOperator_i (PdfOperator::Operands& operands)
				{typedef SimpleGenericOperator<mpl::vector1_c<PropertyType, pString>, OPER_i> COper;
				 return shared_ptr<PdfOperator> (new COper (operands)); };

				static shared_ptr<PdfOperator> createOperator_j (PdfOperator::Operands& operands)
				{typedef SimpleGenericOperator<mpl::vector1_c<PropertyType, pString>, OPER_j> COper;
				 return shared_ptr<PdfOperator> (new COper (operands)); };

				static shared_ptr<PdfOperator> createOperator_k (PdfOperator::Operands& operands)
				{typedef SimpleGenericOperator<mpl::vector1_c<PropertyType, pString>, OPER_k> COper;
				 return shared_ptr<PdfOperator> (new COper (operands)); };

				static shared_ptr<PdfOperator> createOperator_l (PdfOperator::Operands& operands)
				{typedef SimpleGenericOperator<mpl::vector1_c<PropertyType, pString>, OPER_l> COper;
				 return shared_ptr<PdfOperator> (new COper (operands)); };

				static shared_ptr<PdfOperator> createOperator_m (PdfOperator::Operands& operands)
				{typedef SimpleGenericOperator<mpl::vector1_c<PropertyType, pString>, OPER_m> COper;
				 return shared_ptr<PdfOperator> (new COper (operands)); };

				static shared_ptr<PdfOperator> createOperator_n (PdfOperator::Operands& operands)
				{typedef SimpleGenericOperator<mpl::vector1_c<PropertyType, pString>, OPER_n> COper;
				 return shared_ptr<PdfOperator> (new COper (operands)); };

				static shared_ptr<PdfOperator> createOperator_q (PdfOperator::Operands& operands)
				{typedef SimpleGenericOperator<mpl::vector0_c<PropertyType>, OPER_q> COper;
				 return shared_ptr<PdfOperator> (new COper (operands)); };

				static shared_ptr<PdfOperator> createOperator_re (PdfOperator::Operands& operands)
				{typedef SimpleGenericOperator<mpl::vector1_c<PropertyType, pString>, OPER_re> COper;
				 return shared_ptr<PdfOperator> (new COper (operands)); };

				static shared_ptr<PdfOperator> createOperator_rg (PdfOperator::Operands& operands)
				{typedef SimpleGenericOperator<mpl::vector1_c<PropertyType, pString>, OPER_rg> COper;
				 return shared_ptr<PdfOperator> (new COper (operands)); };

				static shared_ptr<PdfOperator> createOperator_ri (PdfOperator::Operands& operands)
				{typedef SimpleGenericOperator<mpl::vector1_c<PropertyType, pString>, OPER_ri> COper;
				 return shared_ptr<PdfOperator> (new COper (operands)); };

				static shared_ptr<PdfOperator> createOperator_s (PdfOperator::Operands& operands)
				{typedef SimpleGenericOperator<mpl::vector1_c<PropertyType, pString>, OPER_s> COper;
				 return shared_ptr<PdfOperator> (new COper (operands)); };

				static shared_ptr<PdfOperator> createOperator_sc (PdfOperator::Operands& operands)
				{typedef SimpleGenericOperator<mpl::vector1_c<PropertyType, pString>, OPER_sc> COper;
				 return shared_ptr<PdfOperator> (new COper (operands)); };

				static shared_ptr<PdfOperator> createOperator_scn (PdfOperator::Operands& operands)
				{typedef SimpleGenericOperator<mpl::vector1_c<PropertyType, pString>, OPER_scn> COper;
				 return shared_ptr<PdfOperator> (new COper (operands)); };

				static shared_ptr<PdfOperator> createOperator_sh (PdfOperator::Operands& operands)
				{typedef SimpleGenericOperator<mpl::vector1_c<PropertyType, pString>, OPER_sh> COper;
				 return shared_ptr<PdfOperator> (new COper (operands)); };

				static shared_ptr<PdfOperator> createOperator_v (PdfOperator::Operands& operands)
				{typedef SimpleGenericOperator<mpl::vector1_c<PropertyType, pString>, OPER_v> COper;
				 return shared_ptr<PdfOperator> (new COper (operands)); };

				static shared_ptr<PdfOperator> createOperator_w (PdfOperator::Operands& operands)
				{typedef SimpleGenericOperator<mpl::vector1_c<PropertyType, pString>, OPER_w> COper;
				 return shared_ptr<PdfOperator> (new COper (operands)); };

				static shared_ptr<PdfOperator> createOperator_y (PdfOperator::Operands& operands)
				{typedef SimpleGenericOperator<mpl::vector1_c<PropertyType, pString>, OPER_y> COper;
				 return shared_ptr<PdfOperator> (new COper (operands)); };



	};

	//
	// Table specifying known operators
	//
	const KnownOperators::OperatorTab KnownOperators::tab[] = { 

		{OPER__SLASH, KnownOperators::createOperator__SLASH},
		{OPER__APO, KnownOperators::createOperator__APO},
		{OPER_B, KnownOperators::createOperator_B},
		{OPER_B_STAR, KnownOperators::createOperator_B_STAR},
		{OPER_BDC, KnownOperators::createOperator_BDC},
		{OPER_BI, KnownOperators::createOperator_BI},
		{OPER_BMC, KnownOperators::createOperator_BMC},
		{OPER_BX, KnownOperators::createOperator_BX},
		{OPER_CS, KnownOperators::createOperator_CS},
		{OPER_DP, KnownOperators::createOperator_DP},
		{OPER_Do, KnownOperators::createOperator_Do},
		{OPER_EI, KnownOperators::createOperator_EI},
		{OPER_EMC, KnownOperators::createOperator_EMC},
		{OPER_ET, KnownOperators::createOperator_ET},
		{OPER_EX, KnownOperators::createOperator_EX},
		{OPER_F, KnownOperators::createOperator_F},
		{OPER_G, KnownOperators::createOperator_G},
		{OPER_ID, KnownOperators::createOperator_ID},
		{OPER_J, KnownOperators::createOperator_J},
		{OPER_K, KnownOperators::createOperator_K},
		{OPER_M, KnownOperators::createOperator_M},
		{OPER_MP, KnownOperators::createOperator_MP},
		{OPER_Q, KnownOperators::createOperator_Q},
		{OPER_RG, KnownOperators::createOperator_RG},
		{OPER_S, KnownOperators::createOperator_S},
		{OPER_SC, KnownOperators::createOperator_SC},
		{OPER_SCN, KnownOperators::createOperator_SCN},
		{OPER_T_STAR, KnownOperators::createOperator_T_STAR},
		{OPER_TD, KnownOperators::createOperator_TD},
		{OPER_TJ, KnownOperators::createOperator_TJ},
		{OPER_TL, KnownOperators::createOperator_TL},
		{OPER_Tc, KnownOperators::createOperator_Tc},
		{OPER_Td, KnownOperators::createOperator_Td},
		{OPER_Tf, KnownOperators::createOperator_Tf},
		{OPER_Tj, KnownOperators::createOperator_Tj},
		{OPER_Tm, KnownOperators::createOperator_Tm},
		{OPER_Tr, KnownOperators::createOperator_Tr},
		{OPER_Ts, KnownOperators::createOperator_Ts},
		{OPER_Tw, KnownOperators::createOperator_Tw},
		{OPER_Tz, KnownOperators::createOperator_Tz},
		{OPER_W, KnownOperators::createOperator_W},
		{OPER_W_STAR, KnownOperators::createOperator_W_STAR},
		{OPER_b, KnownOperators::createOperator_b},
		{OPER_b_STAR, KnownOperators::createOperator_b_STAR},
		{OPER_c, KnownOperators::createOperator_c},
		{OPER_cm, KnownOperators::createOperator_cm},
		{OPER_cs, KnownOperators::createOperator_cs},
		{OPER_d, KnownOperators::createOperator_d},
		{OPER_d0, KnownOperators::createOperator_d0},
		{OPER_d1, KnownOperators::createOperator_d1},
		{OPER_f, KnownOperators::createOperator_f},
		{OPER_f_STAR, KnownOperators::createOperator_f_STAR},
		{OPER_g, KnownOperators::createOperator_g},
		{OPER_gs, KnownOperators::createOperator_gs},
		{OPER_h, KnownOperators::createOperator_h},
		{OPER_i, KnownOperators::createOperator_i},
		{OPER_j, KnownOperators::createOperator_j},
		{OPER_k, KnownOperators::createOperator_k},
		{OPER_l, KnownOperators::createOperator_l},
		{OPER_m, KnownOperators::createOperator_m},
		{OPER_n, KnownOperators::createOperator_n},
		{OPER_q, KnownOperators::createOperator_q},
		{OPER_re, KnownOperators::createOperator_re},
		{OPER_rg, KnownOperators::createOperator_rg},
		{OPER_ri, KnownOperators::createOperator_ri},
		{OPER_s, KnownOperators::createOperator_s},
		{OPER_sc, KnownOperators::createOperator_sc},
		{OPER_scn, KnownOperators::createOperator_scn},
		{OPER_sh, KnownOperators::createOperator_sh},
		{OPER_v, KnownOperators::createOperator_v},
		{OPER_w, KnownOperators::createOperator_w},
		{OPER_y, KnownOperators::createOperator_y},
	
	};

	//
	// NEEDS to be AFTER the declaration of tab
	//
	shared_ptr<PdfOperator>
	KnownOperators::findOp (const string& op, PdfOperator::Operands& operands) 
	{
		printDbg (DBG_DBG, "Finding operator: " << op);

		int lo, hi, med, cmp;
		
		cmp = std::numeric_limits<int>::max ();
		lo = -1;
		hi = sizeof (tab) / sizeof (OperatorTab);
		
		printDbg (DBG_DBG, "size: " << hi );
		
		// 
		// dividing of interval
		// 
		while (hi - lo > 1) 
		{
			med = (lo + hi) / 2;
			cmp = op.compare (tab[med].name);
			cmp = strcmp(op.c_str(),tab[med].name);
			printDbg (DBG_DBG, "found " << cmp << " " << op << " " << tab[med].name );
			if (cmp > 0)
				lo = med;
			else if (cmp < 0)
				hi = med;
			else
				lo = hi = med;
		}
		if (0 != cmp)
		{
			printDbg (DBG_DBG, "Operator not found.");
			return shared_ptr<UnknownPdfOperator> (new UnknownPdfOperator (operands, op));
		}
				
		printDbg (DBG_DBG, "Operator found. tab[" << lo << "]");
		// Result in lo
		return tab[lo].func (operands);
  	}

	
	
  /*{"B",   0, {tchkNone},
	  &Gfx::opFillStroke},
  {"B*",  0, {tchkNone},
	  &Gfx::opEOFillStroke},
  {"BDC", 2, {tchkName,   tchkProps},
	  &Gfx::opBeginMarkedContent},
  {"BI",  0, {tchkNone},
          &Gfx::opBeginImage},
  {"BMC", 1, {tchkName},
          &Gfx::opBeginMarkedContent},
  {"BT",  0, {tchkNone},
          &Gfx::opBeginText},
  {"BX",  0, {tchkNone},
          &Gfx::opBeginIgnoreUndef},
  {"CS",  1, {tchkName},
          &Gfx::opSetStrokeColorSpace},
  {"DP",  2, {tchkName,   tchkProps},
          &Gfx::opMarkPoint},
  {"Do",  1, {tchkName},
          &Gfx::opXObject},
  {"EI",  0, {tchkNone},
          &Gfx::opEndImage},
  {"EMC", 0, {tchkNone},
          &Gfx::opEndMarkedContent},
  {"ET",  0, {tchkNone},
          &Gfx::opEndText},
  {"EX",  0, {tchkNone},
          &Gfx::opEndIgnoreUndef},
  {"F",   0, {tchkNone},
          &Gfx::opFill},
  {"G",   1, {tchkNum},
          &Gfx::opSetStrokeGray},
  {"ID",  0, {tchkNone},
          &Gfx::opImageData},
  {"J",   1, {tchkInt},
          &Gfx::opSetLineCap},
  {"K",   4, {tchkNum,    tchkNum,    tchkNum,    tchkNum},
          &Gfx::opSetStrokeCMYKColor},
  {"M",   1, {tchkNum},
          &Gfx::opSetMiterLimit},
  {"MP",  1, {tchkName},
          &Gfx::opMarkPoint},
  {"Q",   0, {tchkNone},
          &Gfx::opRestore},
  {"RG",  3, {tchkNum,    tchkNum,    tchkNum},
          &Gfx::opSetStrokeRGBColor},
  {"S",   0, {tchkNone},
          &Gfx::opStroke},
  {"SC",  -4, {tchkNum,   tchkNum,    tchkNum,    tchkNum},
          &Gfx::opSetStrokeColor},
  {"SCN", -5, {tchkSCN,   tchkSCN,    tchkSCN,    tchkSCN,
	       tchkSCN},
          &Gfx::opSetStrokeColorN},
  {"T*",  0, {tchkNone},
          &Gfx::opTextNextLine},
  {"TD",  2, {tchkNum,    tchkNum},
          &Gfx::opTextMoveSet},
  {"TJ",  1, {tchkArray},
          &Gfx::opShowSpaceText},
  {"TL",  1, {tchkNum},
          &Gfx::opSetTextLeading},
  {"Tc",  1, {tchkNum},
          &Gfx::opSetCharSpacing},
  {"Td",  2, {tchkNum,    tchkNum},
          &Gfx::opTextMove},
  {"Tf",  2, {tchkName,   tchkNum},
          &Gfx::opSetFont},
  {"Tj",  1, {tchkString},
          &Gfx::opShowText},
  {"Tm",  6, {tchkNum,    tchkNum,    tchkNum,    tchkNum,
	      tchkNum,    tchkNum},
          &Gfx::opSetTextMatrix},
  {"Tr",  1, {tchkInt},
          &Gfx::opSetTextRender},
  {"Ts",  1, {tchkNum},
          &Gfx::opSetTextRise},
  {"Tw",  1, {tchkNum},
          &Gfx::opSetWordSpacing},
  {"Tz",  1, {tchkNum},
          &Gfx::opSetHorizScaling},
  {"W",   0, {tchkNone},
          &Gfx::opClip},
  {"W*",  0, {tchkNone},
          &Gfx::opEOClip},
  {"b",   0, {tchkNone},
          &Gfx::opCloseFillStroke},
  {"b*",  0, {tchkNone},
          &Gfx::opCloseEOFillStroke},
  {"c",   6, {tchkNum,    tchkNum,    tchkNum,    tchkNum,
	      tchkNum,    tchkNum},
          &Gfx::opCurveTo},
  {"cm",  6, {tchkNum,    tchkNum,    tchkNum,    tchkNum,
	      tchkNum,    tchkNum},
          &Gfx::opConcat},
  {"cs",  1, {tchkName},
          &Gfx::opSetFillColorSpace},
  {"d",   2, {tchkArray,  tchkNum},
          &Gfx::opSetDash},
  {"d0",  2, {tchkNum,    tchkNum},
          &Gfx::opSetCharWidth},
  {"d1",  6, {tchkNum,    tchkNum,    tchkNum,    tchkNum,
	      tchkNum,    tchkNum},
          &Gfx::opSetCacheDevice},
  {"f",   0, {tchkNone},
          &Gfx::opFill},
  {"f*",  0, {tchkNone},
          &Gfx::opEOFill},
  {"g",   1, {tchkNum},
          &Gfx::opSetFillGray},
  {"gs",  1, {tchkName},
          &Gfx::opSetExtGState},
  {"h",   0, {tchkNone},
          &Gfx::opClosePath},
  {"i",   1, {tchkNum},
          &Gfx::opSetFlat},
  {"j",   1, {tchkInt},
          &Gfx::opSetLineJoin},
  {"k",   4, {tchkNum,    tchkNum,    tchkNum,    tchkNum},
          &Gfx::opSetFillCMYKColor},
  {"l",   2, {tchkNum,    tchkNum},
          &Gfx::opLineTo},
  {"m",   2, {tchkNum,    tchkNum},
          &Gfx::opMoveTo},
  {"n",   0, {tchkNone},
          &Gfx::opEndPath},
  {"q",   0, {tchkNone},
          &Gfx::opSave},
  {"re",  4, {tchkNum,    tchkNum,    tchkNum,    tchkNum},
          &Gfx::opRectangle},
  {"rg",  3, {tchkNum,    tchkNum,    tchkNum},
          &Gfx::opSetFillRGBColor},
  {"ri",  1, {tchkName},
          &Gfx::opSetRenderingIntent},
  {"s",   0, {tchkNone},
          &Gfx::opCloseStroke},
  {"sc",  -4, {tchkNum,   tchkNum,    tchkNum,    tchkNum},
          &Gfx::opSetFillColor},
  {"scn", -5, {tchkSCN,   tchkSCN,    tchkSCN,    tchkSCN,
	       tchkSCN},
          &Gfx::opSetFillColorN},
  {"sh",  1, {tchkName},
          &Gfx::opShFill},
  {"v",   4, {tchkNum,    tchkNum,    tchkNum,    tchkNum},
          &Gfx::opCurveTo1},
  {"w",   1, {tchkNum},
          &Gfx::opSetLineWidth},
  {"y",   4, {tchkNum,    tchkNum,    tchkNum,    tchkNum},
          &Gfx::opCurveTo2},
*/

//==========================================================
} // namespace
//==========================================================




//
// Parse the xpdf object, representing the content stream
//
CContentStream::CContentStream (shared_ptr<IProperty> stream, Object* obj) : contentstream (stream)
{
	// not implemented yet
	assert (obj != NULL);

	printDbg (DBG_DBG, "Creating content stream.");
	
	//
	// Create the parser and lexer and get objects from it
	//
	scoped_ptr<Parser> parser (new Parser (NULL, new Lexer(NULL, obj)));

	PdfOperator::Operands operands;
	
	Object o;
	parser->getObj(&o);

	//
	// Loop through all object, if it is an operator create pdfoperator else assume it is an operand
	//
	while (!o.isEOF()) 
	{
		if (o.isCmd ())
		{
			// Create operator
			operators.push_back (KnownOperators::findOp (string (o.getCmd ()), operands));
			
			assert (operands.empty());
			if (!operands.empty ())
				throw MalformedFormatExeption ("CContentStream::CContentStream() Operands left on stack in pdf content stream after operator.");
				
		}else
		{
			shared_ptr<IProperty> pIp (createObjFromXpdfObj (o));
			operands.push_back (pIp);
		}

		// free it else memory leak
		o.free ();
		// grab the next object
		parser->getObj(&o);
	}
	
}

//
// 
//
void
CContentStream::getStringRepresentation (string& str) const
{
	printDbg (DBG_DBG, " ()");
	string frst, lst, tmp;

	str = "";
	for (Operators::const_iterator it = operators.begin (); it != operators.end(); ++it)
	{
			
		(*it)->getOperatorName (frst, lst);
		printDbg (DBG_DBG, "Operator name: " << frst << " " << lst << " param count: " << (*it)->getParametersCount() );
		
		(*it)->getStringRepresentation (tmp);
		str += tmp + "\n";
		tmp = "";
	}
}

//==========================================================
} // namespace pdfobjects
//==========================================================
