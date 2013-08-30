#include "TerseEncoder.h"

#include <ace/UUID.h>

#include <log.h>
#include "json/json.h"
#include "protocol/AmmoMessages.pb.h"

#include "BinaryOutputStream.h"

using namespace std;

const std::string PLI_MIME_TYPE = "ammo/transapps.pli.locations";
const std::string CHAT_MIME_TYPE = "ammo/transapps.chat.message_groupAll";

TerseEncoder::TerseEncoder() {

}

TerseEncoder::~TerseEncoder() {

}

bool TerseEncoder::encodeTerseFromJson(const std::string &mimeType, const std::string &jsonData, std::string &terseOutput) {
  if(mimeType == PLI_MIME_TYPE) {
    return encodePli(jsonData, terseOutput);
  } else if(mimeType == CHAT_MIME_TYPE) {
    return encodePli(jsonData, terseOutput);
  } else {
    LOG_ERROR("Received unknown mime type: " << mimeType);
    return false;
  }
}

bool TerseEncoder::encodePli(const std::string &jsonData, std::string &terseOutput) {
/*
   * Terse PLI type definition:
   *   Name : string (uint16_t size + UTF-8 data)
   *   Lat  : int32
   *   Lon  : int32
   *   createdTime: uint32; time from json / 1000
   *   GroupPliBlob : blob
   */
  Json::Value root;
  Json::Reader reader;

  //We only parse the c_str component of the data because the data may contain
  //binary data at the end (separated by a null, which is why this works).
  bool parsingSuccessful = reader.parse(jsonData.c_str(), root);

  if(parsingSuccessful) {
    string name = "";
    int32_t lat = 0;
    int32_t lon = 0;
    uint32_t time = 0;
    
    if(root["name"].isString()) {
      name = root["name"].asString();
    } else {
      LOG_ERROR("Received invalid PLI message from gateway... 'name' is not a string");
      return false;
    }
    
    if(root["lat"].isString()) {
      lat = atoi(root["lat"].asString().c_str());
    } else {
      LOG_ERROR("Received invalid PLI message from gateway... 'lat' is not a string");
      return false;
    }
    
    if(root["lon"].isString()) {
      lon = atoi(root["lon"].asString().c_str());
    } else {
      LOG_ERROR("Received invalid PLI message from gateway... 'lon' is not a string");
      return false;
    }
    
    if(root["created"].isString()) {
      time = atoll(root["created"].asString().c_str()) / 1000;
    } else {
      LOG_ERROR("Received invalid PLI message from gateway... 'created' is not a string");
      return false;
    }
    LOG_DEBUG("XXX name=" << name << " lat=" << lat << " lon=" << lon << " time=" << time);

    //Pass through PLI messages 1:1
    BinaryOutputStream tersePayload(BinaryOutputStream::ORDER_BIG_ENDIAN);
    tersePayload.appendTerseString(name);
    tersePayload.appendInt32(lat);
    tersePayload.appendInt32(lon);
    tersePayload.appendUInt32(time);

    //TODO: parse and forward PLI relay, too
    std::string pliRelayBlob("\0", 1);
    tersePayload.appendTerseBlob(pliRelayBlob);

    ammo::protocol::MessageWrapper msg;
    msg.set_type(ammo::protocol::MessageWrapper_MessageType_TERSE_MESSAGE);
    msg.set_message_priority(0);
    ammo::protocol::TerseMessage *terseMsg = msg.mutable_terse_message();
    terseMsg->set_mime_type(5);
    terseMsg->set_data(tersePayload.getString());

    terseOutput = msg.SerializeAsString();
    return true;
  } else {
    LOG_ERROR("Couldn't parse PLI data from gateway");
    return false;
  }
}

bool TerseEncoder::encodeChat(const std::string &jsonData, std::string &terseOutput) {
  /* Terse Chat type definition:
   *   Originator : string
   *   Text : string
   *   Created Date : int64
   */
  Json::Value root;
  Json::Reader reader;

  //We only parse the c_str component of the data because the data may contain
  //binary data at the end (separated by a null, which is why this works).
  bool parsingSuccessful = reader.parse(jsonData.c_str(), root);

  if(parsingSuccessful) {
    string originator = "";
    string text = "";
    int64_t time = 0;
    
    if(root["originator"].isString()) {
      originator = root["originator"].asString();
    } else {
      LOG_ERROR("Received invalid chat message from gateway... 'originator' is not a string");
      return false;
    }
    
    if(root["text"].isString()) {
      text = root["text"].asString();
    } else {
      LOG_ERROR("Received invalid chat message from gateway... 'text' is not a string");
      return false;
    }
    
    if(root["created_date"].isString()) {
      time = atoll(root["created_date"].asString().c_str());
    } else {
      LOG_ERROR("Received invalid chat message from gateway... 'time' is not a string");
      return false;
    }
    ACE_Utils::UUID *uuid = ACE_Utils::UUID_GENERATOR::instance ()->generate_UUID ();


    BinaryOutputStream tersePayload;
    tersePayload.appendTerseString(string(uuid->to_string()->c_str()));
    tersePayload.appendTerseString(originator);
    tersePayload.appendTerseString(text);
    tersePayload.appendInt32(time / 1000);

    ammo::protocol::MessageWrapper msg;
    msg.set_type(ammo::protocol::MessageWrapper_MessageType_TERSE_MESSAGE);
    msg.set_message_priority(0);
    ammo::protocol::TerseMessage *terseMsg = msg.mutable_terse_message();
    terseMsg->set_mime_type(4);
    terseMsg->set_data(tersePayload.getString());

    std::string terseOutput = msg.SerializeAsString();
    return true;
  } else {
    LOG_ERROR("Couldn't parse Chat data from gateway");
    return false;
  }
}