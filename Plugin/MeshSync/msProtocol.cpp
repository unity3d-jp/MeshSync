#include "pch.h"
#include "msProtocol.h"
#include "msFoundation.h"

namespace ms {

Message::~Message()
{
}
void Message::serialize(std::ostream& os) const
{
    write(os, protocol_version);
    write(os, session_id);
    write(os, message_id);
}
void Message::deserialize(std::istream& is)
{
    read(is, protocol_version);
    if (protocol_version != msProtocolVersion) {
        throw std::runtime_error("protocol version not matched");
    }
    read(is, session_id);
    read(is, message_id);
}

GetMessage::GetMessage()
{
}
void GetMessage::serialize(std::ostream& os) const
{
    super::serialize(os);
    write(os, flags);
    write(os, scene_settings);
    write(os, refine_settings);
}
void GetMessage::deserialize(std::istream& is)
{
    super::deserialize(is);
    read(is, flags);
    read(is, scene_settings);
    read(is, refine_settings);
}


SetMessage::SetMessage()
{
}
void SetMessage::serialize(std::ostream& os) const
{
    super::serialize(os);
    scene.serialize(os);
}
void SetMessage::deserialize(std::istream& is)
{
    super::deserialize(is);
    scene.deserialize(is);
}


DeleteMessage::DeleteMessage()
{
}
void DeleteMessage::serialize(std::ostream& os) const
{
    super::serialize(os);
    write(os, entities);
    write(os, materials);
}
void DeleteMessage::deserialize(std::istream& is)
{
    super::deserialize(is);
    read(is, entities);
    read(is, materials);
}


FenceMessage::~FenceMessage() {}
void FenceMessage::serialize(std::ostream& os) const
{
    super::serialize(os);
    write(os, type);
}
void FenceMessage::deserialize(std::istream& is)
{
    super::deserialize(is);
    read(is, type);
}

TextMessage::~TextMessage() {}
void TextMessage::serialize(std::ostream& os) const
{
    super::serialize(os);
    write(os, text);
    write(os, type);
}
void TextMessage::deserialize(std::istream& is)
{
    super::deserialize(is);
    read(is, text);
    read(is, type);
}


ScreenshotMessage::ScreenshotMessage() {}
void ScreenshotMessage::serialize(std::ostream& os) const { super::serialize(os); }
void ScreenshotMessage::deserialize(std::istream& is) { super::deserialize(is); }


QueryMessage::QueryMessage()
{
}

void QueryMessage::serialize(std::ostream & os) const
{
    super::serialize(os);
    write(os, type);
}

void QueryMessage::deserialize(std::istream & is)
{
    super::deserialize(is);
    read(is, type);
}

ResponseMessage::ResponseMessage()
{
}

void ResponseMessage::serialize(std::ostream & os) const
{
    super::serialize(os);
    write(os, text);
}

void ResponseMessage::deserialize(std::istream & is)
{
    super::deserialize(is);
    read(is, text);
}


PollMessage::PollMessage()
{}

void PollMessage::serialize(std::ostream& os) const
{
    super::serialize(os);
    write(os, type);
}

void PollMessage::deserialize(std::istream& is)
{
    super::deserialize(is);
    read(is, type);
}


} // namespace ms
