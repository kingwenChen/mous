#ifndef MOUS_MEDIAITEM_H
#define MOUS_MEDIAITEM_H

#include <inttypes.h>
#include <string>

namespace mous {

struct MediaItem
{
    std::string url;
    bool hasRange;
    uint64_t msBeg;
    uint64_t msEnd;

    std::string title;
    std::string artist;
    std::string album;
    std::string comment;
    std::string genre;
    int32_t year;
    int32_t track;

    MediaItem():
	year(-1),
	track(-1)
    {

    }
};

}
#endif