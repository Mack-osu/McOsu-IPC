#include "McOsuScorePoster.h"

#include <curl.h>
#include <sstream>
#include "OsuScore.h"
#include "OsuBeatmap.h"
#include "json.hpp"

using json = nlohmann::json;

static const char* GradeToString(OsuScore::GRADE grade);
static std::string BuildScoreJson(OsuDatabase::Score& s, OsuDatabaseBeatmap& b, McOsuScorePoster::AdditionalScoreData& d);
static bool PostJson(const std::string& url, const std::string& json);

void McOsuScorePoster::Init()
{
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

void McOsuScorePoster::Shutdown()
{
    curl_global_cleanup();
}

bool McOsuScorePoster::PostScore(const std::string& url, OsuDatabase::Score& score, OsuDatabaseBeatmap& beatmap, AdditionalScoreData& additionalScoreData)
{
    return PostJson(url, BuildScoreJson(score, beatmap, additionalScoreData));
}

static const char* GradeToString(OsuScore::GRADE grade)
{
    switch (grade)
    {
        case OsuScore::GRADE::GRADE_XH: return "XH";
        case OsuScore::GRADE::GRADE_SH: return "SH";
        case OsuScore::GRADE::GRADE_X:  return "X";
        case OsuScore::GRADE::GRADE_S:  return "S";
        case OsuScore::GRADE::GRADE_A:  return "A";
        case OsuScore::GRADE::GRADE_B:  return "B";
        case OsuScore::GRADE::GRADE_C:  return "C";
        case OsuScore::GRADE::GRADE_D:  return "D";
        case OsuScore::GRADE::GRADE_F:  return "F";
        case OsuScore::GRADE::GRADE_N:  return "N";
        default:                        return "N";
    }
}

static std::string BuildScoreJson(OsuDatabase::Score& s, OsuDatabaseBeatmap& b, McOsuScorePoster::AdditionalScoreData& d)
{
    json j;

    j["schema"]    = "mcosu.events.score.v1";
    j["event"]     = "score_set";
    j["timestamp"] = s.unixTimestamp;

    j["score"] =
    {
		{ "grade", GradeToString(d.grade)},
        { "value", s.score },
        { "pp", s.pp },
		{ "pp_fc", d.pp_fc},
		{ "pp_max", d.pp_max},
		{ "accuracy", d.accuracy},


        { "combo",
			{
				{ "max", s.comboMax },
            	{ "map_max", s.maxPossibleCombo },
				{ "perfect", s.perfect }
			}
        },

        { "hits",
			{
				{ "300", s.num300s },
				{ "100", s.num100s },
				{ "50", s.num50s },
				{ "geki", s.numGekis },
				{ "katu", s.numKatus },
				{ "miss", s.numMisses },
				{ "slider_breaks", s.numSliderBreaks }
			}
        },

        { "unstable_rate", s.unstableRate },
        { "hit_error",
			{
				{ "min", s.hitErrorAvgMin },
				{ "max", s.hitErrorAvgMax }
			}
        },

        { "legacy", s.isLegacyScore },
        { "unranked", s.isImportedLegacyScore }
    };

    j["mods"] =
    {
        { "legacy", s.modsLegacy },
		{ "string", d.mod_string},
        { "experimental", s.experimentalModsConVars.toUtf8() }
    };

    j["difficulty"] =
    {
        { "attributes",
			{
				{ "ar", s.AR },
				{ "cs", s.CS },
				{ "od", s.OD },
				{ "hp", s.HP }
			}
        },

        { "stars",
			{
				{ "total", s.starsTomTotal },
				{ "aim", s.starsTomAim },
				{ "speed", s.starsTomSpeed }
			}
        },

        { "speed_multiplier", s.speedMultiplier }
    };

    j["beatmap"] =
    {
        { "md5", b.getMD5Hash() },

        { "title", b.getTitle().toUtf8() },
        { "artist", b.getArtist().toUtf8() },
        { "creator", b.getCreator().toUtf8() },
        { "difficulty", b.getDifficultyName().toUtf8() },

        { "length_ms", b.getLengthMS() },

        { "bpm",
			{
				{ "min", b.getMinBPM() },
				{ "max", b.getMaxBPM() },
				{ "common", b.getMostCommonBPM() }
			}
        },

        { "objects",
			{
				{ "total", b.getNumObjects() },
				{ "circles", b.getNumCircles() },
				{ "sliders", b.getNumSliders() },
				{ "spinners", b.getNumSpinners() }
			}
        },

        { "ids",
			{
				{ "beatmap", b.getID() },
            	{ "beatmapset", b.getSetID() }
			}
        }
    };

    j["player"] =
    {
		{"name", s.playerName.toUtf8()}
    };

    return j.dump();
}

static bool PostJson(const std::string& url, const std::string& json)
{
    CURL* curl = curl_easy_init();
    if (!curl)
        return false;

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, json.size());

    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, 500L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 1000L);

    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);

    CURLcode res = curl_easy_perform(curl);

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    return res == CURLE_OK;
}
