#ifndef COMMON_OFX_H
#define COMMON_OFX_H

#include <string>

#define OFX_HOST_NATRON "fr.inria.Natron"
#define OFX_HOST_NUKE "uk.co.thefoundry.nuke"
#define OFX_HOST_RESOLVE "DaVinciResolve"
#define OFX_HOST_RESOLVE_LITE "DaVinciResolveLite"
#define OFX_HOST_FUSION "com.blackmagicdesign.Fusion"
#define OFX_HOST_VEGAS "com.sonycreativesoftware.vegas"
#define OFX_HOST_CATALYST "com.sony.Catalyst.Edit"

class CommonOFX
{
public:
    static bool isFusion(const std::string host)
    {
        if (host == OFX_HOST_RESOLVE ||
            host == OFX_HOST_RESOLVE_LITE ||
            host == OFX_HOST_FUSION)
        { return true; }
        return false;
    }
    static bool isResolve(const std::string host)
    {
        if (host == OFX_HOST_RESOLVE ||
            host == OFX_HOST_RESOLVE_LITE)
        { return true; }
        return false;
    }
    static bool isNuke(const std::string host)
    {
        if (host == OFX_HOST_NUKE) { return true; }
        return false;
    }
    static bool isVegas(const std::string host)
    {
        if (host == OFX_HOST_VEGAS) { return true; }
        return false;
    }
    static bool hostSupportsRoD(const std::string host)
    {
        if ( CommonOFX::isFusion(host) || CommonOFX::isVegas(host) ) { return false; }
        return true;
    }
};

#endif // COMMON_OFX_H
