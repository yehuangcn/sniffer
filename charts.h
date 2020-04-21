#ifndef CHARTS_H
#define CHARTS_H

#include <string>
#include <vector>
#include <queue>
#include <map>

#include "sql_db.h"
#include "calltable.h"
#include "tools_global.h"


using namespace std;


enum eChartType {
	_chartType_na		= 0,
	_chartType_total	= 1,
	_chartType_count,
	_chartType_cps,
	_chartType_minutes,
	_chartType_count_perc_short,
	_chartType_mos,
	_chartType_mos_caller,
	_chartType_mos_called,
	_chartType_mos_xr_avg,
	_chartType_mos_xr_avg_caller,
	_chartType_mos_xr_avg_called,
	_chartType_mos_xr_min,
	_chartType_mos_xr_min_caller,
	_chartType_mos_xr_min_called,
	_chartType_mos_silence_avg,
	_chartType_mos_silence_avg_caller,
	_chartType_mos_silence_avg_called,
	_chartType_mos_silence_min,
	_chartType_mos_silence_min_caller,
	_chartType_mos_silence_min_called,
	_chartType_mos_lqo_caller,
	_chartType_mos_lqo_called,
	_chartType_packet_lost,
	_chartType_packet_lost_caller,
	_chartType_packet_lost_called,
	_chartType_jitter,
	_chartType_delay,
	_chartType_rtcp_avgjitter,
	_chartType_rtcp_maxjitter,
	_chartType_rtcp_avgfr,
	_chartType_rtcp_maxfr,
	_chartType_silence,
	_chartType_silence_caller,
	_chartType_silence_called,
	_chartType_silence_end,
	_chartType_silence_end_caller,
	_chartType_silence_end_called,
	_chartType_clipping,
	_chartType_clipping_caller,
	_chartType_clipping_called,
	_chartType_pdd,
	_chartType_acd_avg,
	_chartType_acd,
	_chartType_asr_avg,
	_chartType_asr,
	_chartType_ner_avg,
	_chartType_ner,
	_chartType_sipResp,
	_chartType_sipResponse,
	_chartType_sipResponse_base,
	_chartType_codecs,
	_chartType_IP_src,
	_chartType_IP_dst,
	_chartType_domain_src,
	_chartType_domain_dst
};

enum eChartSubType {
	_chartSubType_na = 0,
	_chartSubType_count = 1,
	_chartSubType_value = 2,
	_chartSubType_acd_asr = 3,
	_chartSubType_area = 4,
	_chartSubType_perc = 5
};

enum eChartPercType {
	_chartPercType_NA,
	_chartPercType_Asc,
	_chartPercType_Desc 
};

struct sChartTypeDef {
	u_int16_t chartType;
	u_int8_t pool;
	u_int8_t enableZero;
	eChartPercType percType;
	u_int8_t condEqLeft;
	eChartSubType subType;
};


class cChartDataItem {
public:
	cChartDataItem();
	void add(Call *call, unsigned call_interval, bool firstInterval, bool lastInterval, bool beginInInterval,
		 class cChartSeries *series, class cChartIntervalSeriesData *intervalSeries,
		 u_int32_t calldate_from, u_int32_t calldate_to);
	string json(class cChartSeries *series);
private:
	volatile double max;
	volatile double min;
	volatile double sum;
	vector<float> values;
	volatile unsigned int count;
	map<unsigned int, unsigned int> count_intervals;
	volatile unsigned int countAll;
	volatile unsigned int countConected;
	volatile unsigned int sumDuration;
	volatile unsigned int countShort;
};


class cChartDataMultiseriesItem {
public:
	cChartDataMultiseriesItem();
	~cChartDataMultiseriesItem();
	string json(class cChartSeries *series, class cChartIntervalSeriesData *intervalSeries);
private:
	map<int, cChartDataItem*> data;
friend class cChartIntervalSeriesData;
};


class cChartDataPool {
public:
	cChartDataPool();
	~cChartDataPool();
	void createPool(u_int32_t timeFrom, u_int32_t timeTo);
	void initPoolRslt();
	void add(Call *call, unsigned call_interval, bool firstInterval, bool lastInterval, bool beginInInterval,
		 class cChartSeries *series, class cChartInterval *interval,
		 u_int32_t calldate_from, u_int32_t calldate_to);
	string json(class cChartSeries *series, class cChartInterval *interval);
private:
	volatile unsigned int all;
	map<unsigned int, unsigned int> all_intervals;
	volatile unsigned int all_fi;
	volatile unsigned int all_li;
	volatile u_int32_t *pool;
};


class cChartIntervalSeriesData {
public:
	cChartIntervalSeriesData(class cChartSeries *series = NULL, class cChartInterval *interval = NULL);
	~cChartIntervalSeriesData();
	void prepareData();
	void add(Call *call, unsigned call_interval, bool firstInterval, bool lastInterval, bool beginInInterval,
		 u_int32_t calldate_from, u_int32_t calldate_to);
	void store(class cChartInterval *interval, SqlDb *sqlDb);
	void lock_data() { __SYNC_LOCK(sync_data); }
	void unlock_data() { __SYNC_UNLOCK(sync_data); }
private:
	class cChartSeries *series;
	class cChartInterval *interval;
	cChartDataItem *dataItem;
	cChartDataPool *dataPool;
	cChartDataMultiseriesItem *dataMultiseriesItem;
	vector<string> param;
	map<string_icase, int> param_map;
	volatile int sync_data;
friend class cChartDataItem;
friend class cChartDataMultiseriesItem;
friend class cChartInterval;
};


class cChartInterval {
public:
	cChartInterval();
	~cChartInterval();
	void setInterval(u_int32_t timeFrom, u_int32_t timeTo);
	void add(Call *call, unsigned call_interval, bool firstInterval, bool lastInterval, bool beginInInterval,
		 u_int32_t calldate_from, u_int32_t calldate_to,
		 map<class cChartFilter*, bool> *filters_map);
	void store(u_int32_t act_time, u_int32_t real_time, SqlDb *sqlDb);
	void init();
	void clear();
private:
	u_int32_t timeFrom;
	u_int32_t timeTo;
	map<string, cChartIntervalSeriesData*> seriesData;
	u_int32_t last_store_at;
	u_int32_t last_store_at_real;
	volatile u_int32_t counter_add;
friend class cChartDataPool;
friend class cChartIntervalSeriesData;
friend class cCharts;
};


class cChartFilter {
public:
	cChartFilter(const char *filter, const char *filter_only_sip_ip, const char *filter_without_sip_ip);
	~cChartFilter();
	bool check(Call *call, void *callData, class cFiltersCache *filtersCache);
private:
	string filter;
	string filter_only_sip_ip;
	string filter_without_sip_ip;
	cEvalFormula::sSplitOperands *filter_s;
	cEvalFormula::sSplitOperands *filter_only_sip_ip_s;
	cEvalFormula::sSplitOperands *filter_without_sip_ip_s;
	bool ip_filter_contain_sipcallerip;
	bool ip_filter_contain_sipcalledip;
	volatile int used_counter;
friend class cChartSeries;
friend class cCharts;
};

class cChartNerLsrFilter {
private:
	struct sFilterItem {
		sFilterItem(unsigned lsr) {
			this->lsr = lsr;
		}
		bool check(unsigned lsr) {
			while(log10(lsr) > log10(this->lsr)) {
				lsr = lsr / 10;
			}
			return(lsr == this->lsr);
		}
		unsigned lsr;
	};
public:
	void parseData(JsonItem *jsonData);
	bool check(unsigned lsr) {
		if(b.size()) {
			for(unsigned i = 0; i < b.size(); i++) {
				if(b[i].check(lsr)) {
					return(false);
				}
			}
		}
		if(w.size()) {
			for(unsigned i = 0; i < w.size(); i++) {
				if(w[i].check(lsr)) {
					return(true);
				}
			}
		}
		return(false);
	}
private:
	vector<sFilterItem> w;
	vector<sFilterItem> b;
};

class cChartSeries {
public:
	cChartSeries(unsigned int id, const char *config_id, const char *config, cCharts *charts);
	~cChartSeries();
	void clear();
	bool isIntervals() { 
		return(intervals.size() > 0);
	}
	bool isArea() { 
		return(def.subType == _chartSubType_area); 
	}
	bool checkFilters(map<class cChartFilter*, bool> *filters_map);
private:
	unsigned int id;
	string config_id;
	string type_source;
	string chartType;
	vector<double> intervals;
	vector<string> param;
	map<string_icase, int> param_map;
	vector<cChartFilter*> filters;
	cChartNerLsrFilter *ner_lsr_filter;
	sChartTypeDef def;
	volatile int used_counter;
friend class cChartDataItem;
friend class cChartDataPool;
friend class cChartIntervalSeriesData;
friend class cChartInterval;
friend class cCharts;
};

class cCharts {
public:
	cCharts();
	~cCharts();
	void load(SqlDb *sqlDb);
	void reload();
	void clear();
	cChartFilter* getFilter(const char *filter, bool enableAdd, 
				const char *filter_only_sip_ip, const char *filter_without_sip_ip);
	cChartFilter* addFilter(const char *filter, const char *filter_only_sip_ip, const char *filter_without_sip_ip);
	void add(Call *call, void *callData, class cFiltersCache *filtersCache);
	void checkFilters(Call *call, void *callData, map<cChartFilter*, bool> *filters, class cFiltersCache *filtersCache);
	void store(bool forceAll = false);
	void cleanup();
	bool seriesIsUsed(const char *config_id);
	void lock_intervals() { __SYNC_LOCK(sync_intervals); }
	void unlock_intervals() { __SYNC_UNLOCK(sync_intervals); }
private:
	map<string, cChartSeries*> series;
	map<u_int32_t, cChartInterval*> intervals;
	map<string, cChartFilter*> filters;
	u_int32_t first_interval;
	u_int32_t last_interval;
	unsigned maxValuesPartsForPercentile;
	unsigned maxLengthSipResponseText;
	unsigned intervalStorePeriod;
	unsigned intervalExpiration;
	unsigned intervalReload;
	SqlDb *sqlDbStore;
	u_int32_t last_store_at;
	u_int32_t last_store_at_real;
	u_int32_t last_cleanup_at;
	u_int32_t last_reload_at;
	volatile int sync_intervals;
friend class cChartDataItem;
friend class cChartInterval;
friend class Call;
};

class cFilterCacheItem {
public:
	inline cFilterCacheItem(unsigned limit);
	inline int get(vmIP *ip);
	inline void add(vmIP *ip, bool set);
	inline int get(vmIP *ip1, vmIP *ip2);
	inline void add(vmIP *ip1, vmIP *ip2, bool set);
private:
	unsigned limit;
	queue<vmIP> ip_queue;
	map<vmIP, bool> ip_map;
	queue<d_item<vmIP> > ip2_queue;
	map<d_item<vmIP>, bool> ip2_map;
};

class cFiltersCache {
public:
	cFiltersCache(unsigned limit, unsigned limit2);
	~cFiltersCache();
	int get(cChartFilter *filter, vmIP *ip);
	void add(cChartFilter *filter, vmIP *ip, bool set);
	int get(cChartFilter *filter, vmIP *ip1, vmIP *ip2);
	void add(cChartFilter *filter, vmIP *ip1, vmIP *ip2, bool set);
private:
	unsigned limit, limit2;
	map<cChartFilter*, cFilterCacheItem*> cache_map;
};

void chartsCacheInit(SqlDb *sqlDb);
void chartsCacheTerm();
bool chartsCacheIsSet();
void chartsCacheAddCall(Call *call, void *callData, cFiltersCache *filtersCache);
void chartsCacheStore(bool forceAll = false);
void chartsCacheCleanup();
void chartsCacheReload();


#endif //CHARTS_H
