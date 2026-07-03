#include "PyFacade/FacadeTypes.h"

namespace PyFacade
{
    std::string entityTypeToString(Eg::EType type)
    {
        switch (type)
        {
            case Eg::EType::POINT: return "point";
            case Eg::EType::LINE: return "line";
            case Eg::EType::POLYGON: return "polygon";
            case Eg::EType::ARC: return "arc";
            case Eg::EType::CIRCLE: return "circle";
            case Eg::EType::ELLIPSE: return "ellipse";
            case Eg::EType::BEZIER2: return "bezier2";
            case Eg::EType::BEZIER: return "bezier";
            case Eg::EType::SPLINE: return "spline";
            case Eg::EType::SMARTLINE: return "smartline";
            case Eg::EType::TEXT: return "text";
            case Eg::EType::BAR_CODE: return "bar_code";
            case Eg::EType::QR_CODE: return "qr_code";
            case Eg::EType::IMAGE: return "image";
            default: return "unknown";
        }
    }
}