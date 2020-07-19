#ifndef COMMON_OFX_H
#define COMMON_OFX_H

#include <string>
#include <fstream>
#include <stdlib.h>

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
    static bool isFusion(const std::string &host)
    {
        if (host == OFX_HOST_FUSION) { return true; }
        return false;
    }
    static bool isResolve(const std::string &host)
    {
        if (host == OFX_HOST_RESOLVE ||
            host == OFX_HOST_RESOLVE_LITE)
        { return true; }
        return false;
    }
    static bool isBMD(const std::string &host)
    {
        return ( isFusion(host) || isResolve(host) );
    }
    static bool isNuke(const std::string &host)
    {
        if (host == OFX_HOST_NUKE) { return true; }
        return false;
    }
    static bool isVegas(const std::string &host)
    {
        if (host == OFX_HOST_VEGAS) { return true; }
        return false;
    }
    static bool isBadHost(const std::string &host)
    {
        return ( isNuke(host) || isVegas(host) );
    }
    static const std::string getEnv(const std::string &key)
    {
        char const* val = getenv( key.c_str() );
        return val == nullptr ? std::string() : std::string(val);
    }
    static const std::string getTempPath()
    {
        std::string result;
        result = getEnv("TMP");
        if ( result.empty() ) { result = getEnv("TMPDIR"); }
        if ( result.empty() ) { result = getEnv("TEMP"); }
        if ( result.empty() ) { result = getEnv("TEMPDIR"); }
        if ( result.empty() ) { result = getEnv("USERPROFILE"); }
        if ( result.empty() ) { result = getEnv("HOME"); }
#ifndef _WIN32
        if ( result.empty() ) { result = "/tmp"; }
#endif
        return result;
    }
    static void logToFile(const std::string &content)
    {
        std::ofstream output;
        std::string temp = getTempPath();
        if ( temp.empty() ) { return; }
        temp.append("/TextOFXLog.txt");
        output.open(temp, std::ios_base::app);
        if ( output.fail() ) { return; }
        output << content << "\n";
        output.close();
    }
    static void writeOutput(int width,
                            int height,
                            unsigned char *buffer,
                            float *pixelData)
    {
        if (width == 0 || height == 0 || !buffer) { return; }
        int offset = 0;
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                pixelData[offset + 0] = buffer[offset + 0] * (1.f / 255);
                pixelData[offset + 1] = buffer[offset + 1] * (1.f / 255);
                pixelData[offset + 2] = buffer[offset + 2] * (1.f / 255);
                pixelData[offset + 3] = buffer[offset + 3] * (1.f / 255);
                offset += 4;
            }
        }
    }
    static void writeOutput(int width,
                            int height,
                            unsigned char *buffer,
                            unsigned char *pixelData)
    {
        if (width == 0 || height == 0 || !buffer) { return; }
        int offset = 0;
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                pixelData[offset + 0] = buffer[offset + 0];
                pixelData[offset + 1] = buffer[offset + 1];
                pixelData[offset + 2] = buffer[offset + 2];
                pixelData[offset + 3] = buffer[offset + 3];
                offset += 4;
            }
        }
    }
};

#endif // COMMON_OFX_H
