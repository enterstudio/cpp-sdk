/**
* Copyright 2017 IBM Corp. All Rights Reserved.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
*/

#include "NaturalLanguageUnderstanding.h"

REG_SERIALIZABLE( NaturalLanguageUnderstanding );
RTTI_IMPL( NaturalLanguageUnderstanding, IService );

NaturalLanguageUnderstanding::NaturalLanguageUnderstanding() : IService("NaturalLanguageUnderstandingV1")
{}

//! ISerializable
void NaturalLanguageUnderstanding::Serialize(Json::Value & json)
{
    IService::Serialize(json);
    SerializeVector("m_ReturnParameters", m_ReturnParameters, json);
}

void NaturalLanguageUnderstanding::Deserialize(const Json::Value & json)
{
    IService::Deserialize(json);
    DeserializeVector("m_ReturnParameters", json, m_ReturnParameters);

    if (m_ReturnParameters.size() == 0)
    {
        m_ReturnParameters.push_back("enriched.url.title");
        m_ReturnParameters.push_back("enriched.url.url");
        m_ReturnParameters.push_back("enriched.url.text");
    }
}

//! IService interface
bool NaturalLanguageUnderstanding::Start()
{
    if (!IService::Start())
        return false;

    if (!StringUtil::EndsWith(m_pConfig->m_URL, "api"))
    {
        Log::Error("NaturalLanguageUnderstanding", "Configured URL not ended with api");
        return false;
    }
    if (m_pConfig->m_User.size() == 0)
        Log::Warning("NaturalLanguageUnderstanding", "User id expected in user field.");

    if (m_pConfig->m_Password.size() == 0)
        Log::Warning("NaturalLanguageUnderstanding", "Password expected in password field");

    return true;
}

void NaturalLanguageUnderstanding::GetEntities(const std::string & a_Text, Delegate<const Json::Value &> a_Callback)
{
    std::string parameters = "/v1/analyze";
    parameters += "?version=2017-02-27";
    parameters += "&features=entities";
    parameters += "&language=en";
    parameters += "&text=" + StringUtil::UrlEscape( a_Text );

    Headers headers;
    headers["Content-Type"] = "application/json";

    new RequestJson(this, parameters, "GET", headers, EMPTY_STRING, a_Callback,
                    new CacheRequest( "TextGetRankedNamedEntities", StringHash::DJB(a_Text.c_str()) ) );
}
