/*
 *
 *  Copyright (C) 2010-2012, OFFIS e.V.
 *  All rights reserved.  See COPYRIGHT file for details.
 *
 *  This software and supporting documentation were developed by
 *
 *    OFFIS e.V.
 *    R&D Division Health
 *    Escherweg 2
 *    D-26121 Oldenburg, Germany
 *
 *
 *  Module: dcmsr
 *
 *  Author: Joerg Riesmeier
 *
 *  Purpose:
 *    classes: DSRSpatialCoordinates3DValue
 *
 *  Last Update:      $Author: joergr $
 *  Update Date:      $Date: 2012-06-11 08:53:06 $
 *  CVS/RCS Revision: $Revision: 1.5 $
 *  Status:           $State: Exp $
 *
 *  CVS/RCS Log at end of file
 *
 */


#include "dcmtk/config/osconfig.h"    /* make sure OS specific configuration is included first */

#include "dcmtk/dcmsr/dsrsc3vl.h"
#include "dcmtk/dcmsr/dsrxmld.h"


DSRSpatialCoordinates3DValue::DSRSpatialCoordinates3DValue()
  : GraphicType(DSRTypes::GT3_invalid),
    GraphicDataList(),
    FrameOfReferenceUID(),
    FiducialUID()
{
}


DSRSpatialCoordinates3DValue::DSRSpatialCoordinates3DValue(const DSRTypes::E_GraphicType3D graphicType)
  : GraphicType(graphicType),
    GraphicDataList(),
    FrameOfReferenceUID(),
    FiducialUID()
{
}


DSRSpatialCoordinates3DValue::DSRSpatialCoordinates3DValue(const DSRSpatialCoordinates3DValue &coordinatesValue)
  : GraphicType(coordinatesValue.GraphicType),
    GraphicDataList(coordinatesValue.GraphicDataList),
    FrameOfReferenceUID(coordinatesValue.FrameOfReferenceUID),
    FiducialUID(coordinatesValue.FiducialUID)
{
}


DSRSpatialCoordinates3DValue::~DSRSpatialCoordinates3DValue()
{
}


DSRSpatialCoordinates3DValue &DSRSpatialCoordinates3DValue::operator=(const DSRSpatialCoordinates3DValue &coordinatesValue)
{
    GraphicType = coordinatesValue.GraphicType;
    GraphicDataList = coordinatesValue.GraphicDataList;
    FrameOfReferenceUID = coordinatesValue.FrameOfReferenceUID;
    FiducialUID = coordinatesValue.FiducialUID;
    return *this;
}


void DSRSpatialCoordinates3DValue::clear()
{
    GraphicType = DSRTypes::GT3_invalid;
    GraphicDataList.clear();
    FrameOfReferenceUID.clear();
    FiducialUID.clear();
}


OFBool DSRSpatialCoordinates3DValue::isValid() const
{
    return checkGraphicData(GraphicType, GraphicDataList).good() && checkFrameOfReferenceUID(FrameOfReferenceUID).good();
}


OFBool DSRSpatialCoordinates3DValue::isShort(const size_t flags) const
{
    return GraphicDataList.isEmpty() || ((flags & DSRTypes::HF_renderFullData) == 0);
}


OFCondition DSRSpatialCoordinates3DValue::print(STD_NAMESPACE ostream &stream,
                                                const size_t flags) const
{
    /* GraphicType */
    stream << "(" << DSRTypes::graphicType3DToEnumeratedValue(GraphicType);
    /* ReferencedFrameOfReferenceUID */
    stream << ",";
    if (flags & DSRTypes::PF_printSOPInstanceUID)
        stream << "\"" << FrameOfReferenceUID << "\"";
    /* GraphicData */
    if (!GraphicDataList.isEmpty())
    {
        stream << ",";
        GraphicDataList.print(stream, flags);
    }
    stream << ")";
    return EC_Normal;
}


OFCondition DSRSpatialCoordinates3DValue::readXML(const DSRXMLDocument &doc,
                                                  DSRXMLCursor cursor)
{
    OFCondition result = SR_EC_CorruptedXMLStructure;
    if (cursor.valid())
    {
        cursor.gotoChild();
        /* GraphicData (required) */
        const DSRXMLCursor dataNode = doc.getNamedNode(cursor, "data");
        if (dataNode.valid())
        {
            OFString tmpString;
            /* ReferencedFrameOfReferenceUID (required) */
            doc.getStringFromAttribute(dataNode, FrameOfReferenceUID, "uid");
            /* put value to the graphic data list */
            result = GraphicDataList.putString(doc.getStringFromNodeContent(dataNode, tmpString).c_str());
        }
        /* FiducialUID (optional) */
        const DSRXMLCursor fiducialNode = doc.getNamedNode(cursor, "fiducial", OFFalse /*required*/);
        if (fiducialNode.valid())
            doc.getStringFromAttribute(fiducialNode, FiducialUID, "uid");
    }
    return result;
}


OFCondition DSRSpatialCoordinates3DValue::writeXML(STD_NAMESPACE ostream &stream,
                                                   const size_t flags) const
{
    /* GraphicType is written in TreeNode class */
    if ((flags & DSRTypes::XF_writeEmptyTags) || !GraphicDataList.isEmpty())
    {
        stream << "<data uid=\"" << FrameOfReferenceUID << "\">" << OFendl;
        GraphicDataList.print(stream);
        stream << "</data>" << OFendl;
    }
    if ((flags & DSRTypes::XF_writeEmptyTags) || !FiducialUID.empty())
        stream << "<fiducial uid=\"" << FiducialUID << "\"/>" << OFendl;
    return EC_Normal;
}


OFCondition DSRSpatialCoordinates3DValue::read(DcmItem &dataset)
{
    /* read ReferencedFrameOfReferenceUID */
    OFString tmpString;
    OFCondition result = DSRTypes::getAndCheckStringValueFromDataset(dataset, DCM_ReferencedFrameOfReferenceUID, FrameOfReferenceUID, "1", "1", "SCOORD3D content item");
    /* read GraphicType */
    if (result.good())
        result = DSRTypes::getAndCheckStringValueFromDataset(dataset, DCM_GraphicType, tmpString, "1", "1", "SCOORD3D content item");
    if (result.good())
    {
        GraphicType = DSRTypes::enumeratedValueToGraphicType3D(tmpString);
        /* check GraphicType */
        if (GraphicType == DSRTypes::GT3_invalid)
            DSRTypes::printUnknownValueWarningMessage("GraphicType", tmpString.c_str());
        /* read GraphicData */
        result = GraphicDataList.read(dataset);
        /* read optional attributes */
        if (result.good())
            DSRTypes::getAndCheckStringValueFromDataset(dataset, DCM_FiducialUID, FiducialUID, "1", "3", "SCOORD3D content item");
        /* check GraphicData and report warnings if any */
        checkGraphicData(GraphicType, GraphicDataList, OFTrue /*reportWarnings*/);
    }
    return result;
}


OFCondition DSRSpatialCoordinates3DValue::write(DcmItem &dataset) const
{
    /* write ReferencedFrameOfReferenceUID */
    OFCondition result = DSRTypes::putStringValueToDataset(dataset, DCM_ReferencedFrameOfReferenceUID, FrameOfReferenceUID);
    /* write GraphicType */
    if (result.good())
        DSRTypes::putStringValueToDataset(dataset, DCM_GraphicType, DSRTypes::graphicType3DToEnumeratedValue(GraphicType));
    /* write GraphicData */
    if (result.good())
    {
        if (!GraphicDataList.isEmpty())
            result = GraphicDataList.write(dataset);
    }
    /* write optional attributes */
    if (result.good())
        DSRTypes::putStringValueToDataset(dataset, DCM_FiducialUID, FiducialUID, OFFalse /*allowEmpty*/);
    /* check GraphicData and report warnings if any */
    checkGraphicData(GraphicType, GraphicDataList, OFTrue /*reportWarnings*/);
    return result;
}


OFCondition DSRSpatialCoordinates3DValue::renderHTML(STD_NAMESPACE ostream &docStream,
                                                     STD_NAMESPACE ostream &annexStream,
                                                     size_t &annexNumber,
                                                     const size_t flags) const
{
    /* render GraphicType */
    docStream << DSRTypes::graphicType3DToReadableName(GraphicType);
    /* render GraphicData */
    if (!isShort(flags))
    {
        const char *lineBreak = (flags & DSRTypes::HF_renderSectionTitlesInline) ? " " :
                                (flags & DSRTypes::HF_XHTML11Compatibility) ? "<br />" : "<br>";
        if (flags & DSRTypes::HF_currentlyInsideAnnex)
        {
            docStream << OFendl << "<p>" << OFendl;
            /* render graphic data list (= print)*/
            docStream << "<b>Graphic Data:</b>" << lineBreak;
            GraphicDataList.print(docStream);
            docStream << "</p>";
        } else {
            DSRTypes::createHTMLAnnexEntry(docStream, annexStream, "for more details see", annexNumber, flags);
            annexStream << "<p>" << OFendl;
            /* render graphic data list (= print)*/
            annexStream << "<b>Graphic Data:</b>" << lineBreak;
            GraphicDataList.print(annexStream);
            annexStream << "</p>" << OFendl;
        }
    }
    return EC_Normal;
}


OFCondition DSRSpatialCoordinates3DValue::getValue(DSRSpatialCoordinates3DValue &coordinatesValue) const
{
    coordinatesValue = *this;
    return EC_Normal;
}


OFCondition DSRSpatialCoordinates3DValue::setValue(const DSRSpatialCoordinates3DValue &coordinatesValue,
                                                   const OFBool check)
{
    OFCondition result = EC_Normal;
    if (check)
    {
        /* check whether the passed values are valid */
        result = checkGraphicData(coordinatesValue.GraphicType, coordinatesValue.GraphicDataList);
        if (result.good())
            result = checkFrameOfReferenceUID(coordinatesValue.FrameOfReferenceUID);
        if (result.good())
            result = checkFiducialUID(coordinatesValue.FiducialUID);
    } else {
        /* make sure that the mandatory values are non-empty/invalid */
        if ((coordinatesValue.GraphicType == DSRTypes::GT3_invalid) ||
            coordinatesValue.GraphicDataList.isEmpty() ||
            coordinatesValue.FrameOfReferenceUID.empty())
        {
            result = EC_IllegalParameter;
        }
    }
    if (result.good())
    {
        GraphicType = coordinatesValue.GraphicType;
        GraphicDataList = coordinatesValue.GraphicDataList;
        FrameOfReferenceUID = coordinatesValue.FrameOfReferenceUID;
    }
    return result;
}


OFCondition DSRSpatialCoordinates3DValue::setGraphicType(const DSRTypes::E_GraphicType3D graphicType,
                                                         const OFBool /*check*/)
{
    OFCondition result = EC_IllegalParameter;
    /* check whether the passed value is valid */
    if (graphicType != DSRTypes::GT3_invalid)
    {
        GraphicType = graphicType;
        result = EC_Normal;
    }
    return result;
}


OFCondition DSRSpatialCoordinates3DValue::setFrameOfReferenceUID(const OFString &frameOfReferenceUID,
                                                                 const OFBool check)
{
    OFCondition result = EC_Normal;
    /* check whether the passed value is valid */
    if (check)
        result = checkFrameOfReferenceUID(frameOfReferenceUID);
    else if (frameOfReferenceUID.empty())
        result = EC_IllegalParameter;
    if (result.good())
        FrameOfReferenceUID = frameOfReferenceUID;
    return result;
}


OFCondition DSRSpatialCoordinates3DValue::setFiducialUID(const OFString &fiducialUID,
                                                         const OFBool check)
{
    OFCondition result = EC_Normal;
    /* check whether the passed value is valid */
    if (check)
        result = checkFiducialUID(fiducialUID);
    if (result.good())
        FiducialUID = fiducialUID;
    return result;
}


// helper macro to avoid annoying check of boolean flag
#define REPORT_WARNING(msg) { if (reportWarnings) DCMSR_WARN(msg); }

OFCondition DSRSpatialCoordinates3DValue::checkGraphicData(const DSRTypes::E_GraphicType3D graphicType,
                                                           const DSRGraphicData3DList &graphicDataList,
                                                           const OFBool reportWarnings) const
{
    OFCondition result = SR_EC_InvalidValue;
    // check graphic type and data
    if (graphicType == DSRTypes::GT3_invalid)
        REPORT_WARNING("Invalid GraphicType for SCOORD3D content item")
    else if (graphicDataList.isEmpty())
        REPORT_WARNING("No GraphicData for SCOORD3D content item")
    else
    {
        const size_t count = graphicDataList.getNumberOfItems();
        switch (graphicType)
        {
            case DSRTypes::GT3_Point:
                if (count > 1)
                    REPORT_WARNING("GraphicData has too many entries, only a single entry expected")
                result = EC_Normal;
                break;
            case DSRTypes::GT3_Multipoint:
                if (count < 1)
                    REPORT_WARNING("GraphicData has too few entries, at least one entry expected")
                result = EC_Normal;
                break;
            case DSRTypes::GT3_Polyline:
                if (count < 1)
                    REPORT_WARNING("GraphicData has too few entries, at least one entry expected")
                result = EC_Normal;
                break;
            case DSRTypes::GT3_Polygon:
                if (count < 1)
                    REPORT_WARNING("GraphicData has too few entries, at least one entry expected")
                else
                {
                    if (graphicDataList.getItem(1) != graphicDataList.getItem(count))
                        REPORT_WARNING("First and last entry in GraphicData are not equal (POLYGON)")
                    result = EC_Normal;
                }
                break;
            case DSRTypes::GT3_Ellipse:
                if (count < 4)
                    REPORT_WARNING("GraphicData has too few entries, exactly four entries expected")
                else
                {
                    if (count > 4)
                        REPORT_WARNING("GraphicData has too many entries, exactly four entries expected")
                    result = EC_Normal;
                }
                break;
            case DSRTypes::GT3_Ellipsoid:
                if (count < 6)
                    REPORT_WARNING("GraphicData has too few entries, exactly six entries expected")
                else
                {
                    if (count > 6)
                        REPORT_WARNING("GraphicData has too many entries, exactly six entries expected")
                    result = EC_Normal;
                }
                break;
            default:
                /* GT3_invalid */
                break;
        }
    }
    return result;
}


OFCondition DSRSpatialCoordinates3DValue::checkFrameOfReferenceUID(const OFString &frameOfReferenceUID) const
{
    /* referenced frame of reference UID should never be empty */
    return frameOfReferenceUID.empty() ? SR_EC_InvalidValue
                                       : DcmUniqueIdentifier::checkStringValue(frameOfReferenceUID, "1");
}


OFCondition DSRSpatialCoordinates3DValue::checkFiducialUID(const OFString &fiducialUID) const
{
    /* fiducial UID might be empty */
    return fiducialUID.empty() ? EC_Normal
                               : DcmUniqueIdentifier::checkStringValue(fiducialUID, "1");
}


/*
 *  CVS/RCS Log:
 *  $Log: dsrsc3vl.cc,v $
 *  Revision 1.5  2012-06-11 08:53:06  joergr
 *  Added optional "check" parameter to "set" methods and enhanced documentation.
 *
 *  Revision 1.4  2011-12-16 16:58:34  joergr
 *  Added support for optional attribute Fiducial UID (0070,031A) to SCOORD and
 *  SCOORD3D content items.
 *
 *  Revision 1.3  2010-10-14 13:14:41  joergr
 *  Updated copyright header. Added reference to COPYRIGHT file.
 *
 *  Revision 1.2  2010-09-28 14:15:20  joergr
 *  Removed old CVS log entries (copied from master file).
 *
 *  Revision 1.1  2010-09-28 14:07:29  joergr
 *  Added support for Colon CAD SR which requires a new value type (SCOORD3D).
 *
 *
 */