#ifndef UTIL_MCOSUSCOREPOSTER_H_
#define UTIL_MCOSUSCOREPOSTER_H_

#include <string>
#include "OsuDatabase.h"
#include "OsuDatabaseBeatmap.h"
#include "OsuScore.h"

namespace McOsuScorePoster
{
	struct AdditionalScoreData
	{
		float accuracy;
		OsuScore::GRADE grade;
		double pp_fc;
		double pp_max;
		std::string mod_string;
	};

	void Init();
	void Shutdown();

	bool PostScore(const std::string& url, OsuDatabase::Score& score, OsuDatabaseBeatmap& beatmap, AdditionalScoreData& additionalScoreData);
}


#endif
