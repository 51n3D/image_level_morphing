#include "Matcher.h"

#include <storage/Fingerprint.h>

#include <fstream>
#include <iostream>

#import  "..\MccSdk\MccCOMInterop.tlb" raw_interfaces_only

using namespace processing::storage;
using namespace matching;

Matcher::Matcher()
{
    CoInitialize(nullptr);
}

Matcher::~Matcher()
{
    CoUninitialize();
}

bool Matcher::createTxtMinutiaTemplate(const Fingerprint& fingerprint, const std::string& filename)
{
    std::ofstream f(filename, std::ofstream::out | std::ofstream::trunc);
 
    if (f.is_open())
    {
        f << fingerprint.size().width << std::endl;
        f << fingerprint.size().height << std::endl;
        f << "500" << std::endl; // DPI
        f << fingerprint.getMinutiae().size() << std::endl;
    	
    	for(const auto& minutia: fingerprint.getMinutiae())
    	{
            const auto pos = minutia.getPosition();
            auto dir = minutia.getDirection();

    		// normalizujem smery z [0..2pi] na [-pi..pi]
            dir = fmodf(dir + CV_2PI, CV_2PI);
    		
            f << pos.x << " " << pos.y << " " << dir << std::endl;
    	}
        f.close();
    }
    else
    {
        std::cerr << "Could not write to file! Some error occured!" << std::endl;
        return false;
    }

    return true;
}

double Matcher::match(const std::string& template1, const std::string& template2)
{
    MccCOMInterop::IMatcherPtr pIMatcher(__uuidof(MccCOMInterop::Matcher));

    const auto tpl1 = std::wstring(template1.begin(), template1.end());
    const auto tpl2 = std::wstring(template2.begin(), template2.end());
	
    const auto path1 = SysAllocString(tpl1.c_str());
    const auto path2 = SysAllocString(tpl2.c_str());

    double score = -1;
    pIMatcher->Match(path1, path2, &score);

    return score;
}
