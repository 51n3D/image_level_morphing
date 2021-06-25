#pragma once

#include <string>

namespace processing
{
	namespace storage
	{
		class Fingerprint;
	}
}

namespace matching
{
	/**
	 * \brief Nastroj urceny k ohodnoteniu dvoch odtlackov
	 * odpovedajucim skore podobnosti markantov. Odhad
	 * prebieha prostrednictvom MCC SDK.
	 */
	class Matcher
	{
	private:

		
	public:
		Matcher();
		~Matcher();

		/**
		 * \brief Prevedie sabolonu marknatov vygenerovanu z kniznice Processing
		 * do sablony markantov kompatibiilnej s MCC SDK.
		 * \param fingerprint odtlacok
		 * \param filename nazov suboru kam sa ulozi sablona
		 * \return 
		 */
		static bool createTxtMinutiaTemplate(const processing::storage::Fingerprint& fingerprint, const std::string& filename);

		/**
		 * \brief Ohodnoti 2 sablony markantov proti sebe.
		 * \param template1 cesta k sablone jedna
		 * \param template2 cesta k sablone dva
		 * \return odpovedajuce skore odtlackov
		 */
		double match(const std::string& template1, const std::string& template2);
	};
}
