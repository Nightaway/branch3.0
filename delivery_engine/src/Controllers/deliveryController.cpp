#include "deliveryController.h"

#include <utility/DateTime.h>

#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>

#include "../config.h"
#include "../Utility/DataModel.h"
#include "../Utility/Filter.h"

NS_USING_DRAGON
NS_USING_BOOST

IMPLEMENT_CONTROLLER(Controller, deliveryController)

BEGIN_ACTION_MAP(deliveryController)
	ACTION(deliveryController, ad)
	ACTION(deliveryController, lg)
	ACTION_OP1(deliveryController, ck, "__")
END_ACTION_MAP()

void deliveryController::ad(QueryString &qs)
{
  Log &error = app->GetErrorLog();
  int zoneid = 0;
  try {
    zoneid = lexical_cast<int>(qs["zoneid"]);
  } catch (bad_lexical_cast &e) {
    error.LogFmt("zoneid [%s] %s", qs["zoneid"].c_str(), e.what());
    response->StringResult(" ");
    return;
  }

  NamedSemiSpace space(SHARED_MEM_OBJ_NAME, SHARED_MEM_OBJ_SIZE);
  space.Open();

  ZoneInfo *zoneInfo = GetZoneInfoById(zoneid, space);
  if (zoneInfo == NULL) {
    error.LogFmt("ZoneInfo by id [%d], not found in Shared Memory Object named [%s] size[%d]", 
                  zoneid, SHARED_MEM_OBJ_NAME, SHARED_MEM_OBJ_SIZE);
    response->StringResult(" "); 
    space.Close();
    return ;
  }
  printf("zone id:%d, name:%s\n", zoneInfo->id, zoneInfo->name.c_str());

  std::vector<AdInfo *> adInfos;
  BOOST_FOREACH(int bannerid, zoneInfo->linked_banners)
  {
    printf("linked banner id:%d\n", bannerid);
    AdInfo *adInfo = GetAdInfoById(bannerid, space);
    if (adInfo == NULL)
      continue;

    printf("ad id:%d, instance:%s\n", adInfo->banner_id, adInfo->template_string.c_str());
    
    adInfos.push_back(adInfo);
  }

  Information info = {
    request,
    response,
    &qs,
  };
  filter(adInfos, info, VisitorFilter);

  BOOST_FOREACH(AdInfo *ad, adInfos)
  {
    std::string out = "banenr_id=";
    out += boost::lexical_cast<std::string>(ad->banner_id);
    response->StringResult(out);
    space.Close();
    return ;
  }

  std::string out = "zoneid=";
  out += qs["zoneid"];
  response->StringResult(out);
  space.Close();
  return ;
}

void deliveryController::lg(QueryString &qs)
{
  Log &error = app->GetErrorLog();
  int zoneid = 0, bannerId = 0, campaignid = 0;

  try {
    zoneid = boost::lexical_cast<int>(qs["zoneid"]);
    bannerId = boost::lexical_cast<int>(qs["bannerid"]);
    if (!qs["campaignid"].empty())
      campaignid = boost::lexical_cast<int>(qs["campaignid"]);
  } catch (boost::bad_lexical_cast) {
    error.LogFmt("lg action cast error: zoneid [%s], bannerid [%s], campaignid [%s]", 
          qs["zoneid"].c_str(), qs["bannerid"].c_str(), qs["campaignid"].c_str());
    response->StringResult(" ");
    return ;
  }

  if (!qs["campaignid"].empty()) {
    std::string impc = "impc[" +  qs["campaignid"] + "]";
    response->setCookie(impc, "1", DateTime::YearFromNow(1), "/");
  }

  std::string impb = "impb[" +  qs["bannerid"] + "]";
  response->setCookie(impb, "1", DateTime::YearFromNow(1), "/");

  response->StringResult("lg action");
}

void deliveryController::ck(QueryString &qs)
{
  response->StringResult("ck action");
}
