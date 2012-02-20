#include "CuePack.h"
#include <mous/MediaItem.h>
#include <scx/FileHelp.hpp>
#include <scx/Conv.hpp>
#include <stdio.h>
using namespace scx;

CuePack::CuePack()
{

}

CuePack::~CuePack()
{

}

void CuePack::GetFileSuffix(std::vector<std::string>& list) const
{
    list.clear();
    list.push_back("cue");
}

#include <iostream>
void CuePack::DumpMedia(const std::string& path, std::deque<MediaItem*>& list,
	const std::map<std::string, IMediaPack*>* pMap) const
{
    FILE* file = fopen(path.c_str(), "r");
    Cd* cd = cue_parse_file(file);
    fclose(file);

    string dir = FileDir(path);

    DumpCue(dir, cd, list);
}

void CuePack::DumpStream(const std::string& stream, std::deque<MediaItem*>& list,
	const std::map<std::string, IMediaPack*>* pMap) const
{
    Cd* cd = cue_parse_string(stream.c_str());
    DumpCue("", cd, list);
}

void CuePack::DumpCue(const string& dir, Cd* cd, deque<MediaItem*>& list) const
{
    int ntrack = cd_get_ntrack(cd);

    string album;
    string artist;
    string genre;
    int year = -1;

    char* data = NULL;
    Cdtext* cdt = cd_get_cdtext(cd);
    Rem* rem = cd_get_rem(cd);

    data = cdtext_get(PTI_TITLE, cdt);
    if (data != NULL) {
	album = data;
	delete data;
    }

    data = cdtext_get(PTI_PERFORMER, cdt);
    if (data != NULL) {
	artist = data;
	delete data;
    }

    data = cdtext_get(PTI_GENRE, cdt);
    if (data != NULL) {
	genre = data;
	delete data;
    }

    //cdtext_delete(cdt);
    //rem_free(rem);

    data = rem_get(REM_DATE, rem);
    if (data != NULL) {
	year = StrToNum<int>(data);
	delete data;
    }

    for (int i = 1; i <= ntrack; ++i) {
	MediaItem* item = new MediaItem;
	list.push_back(item);

	Track* track = cd_get_track(cd, i);
	item->url = dir + track_get_filename(track);
	item->hasRange = true;
	item->msBeg = (track_get_start(track))/75*1000;
	item->msEnd = item->msBeg + ((uint64_t)track_get_length(track))/75*1000;
	if (item->msBeg == item->msEnd && i == ntrack)
	    item->msEnd = -1;

	Cdtext* text = track_get_cdtext(track);

	item->album = album;
	item->year = year;

	data = cdtext_get(PTI_TITLE, text);
	if (data != NULL) {
	    item->title = data;
	    delete data;
	}

	data = cdtext_get(PTI_PERFORMER, text);
	if (data != NULL) {
	    item->artist = data;
	    delete data;
	} else {
	    item->artist = artist;
	}

	data = cdtext_get(PTI_GENRE, text);
	if (data != NULL) {
	    item->genre = data;
	    delete data;
	} else {
	    item->genre = genre;
	}

	item->track = i;

	//cdtext_delete(text);

	cout << i << '\t' << item->url << endl;
	cout << "range:" << item->msBeg << "-" << item->msEnd << endl;
    }
}