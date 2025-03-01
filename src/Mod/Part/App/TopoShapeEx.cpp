/****************************************************************************
 *   Copyright (c) 2018 Zheng Lei (realthunder) <realthunder.dev@gmail.com> *
 *                                                                          *
 *   This file is part of the FreeCAD CAx development system.               *
 *                                                                          *
 *   This library is free software; you can redistribute it and/or          *
 *   modify it under the terms of the GNU Library General Public            *
 *   License as published by the Free Software Foundation; either           *
 *   version 2 of the License, or (at your option) any later version.       *
 *                                                                          *
 *   This library  is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *   GNU Library General Public License for more details.                   *
 *                                                                          *
 *   You should have received a copy of the GNU Library General Public      *
 *   License along with this library; see the file COPYING.LIB. If not,     *
 *   write to the Free Software Foundation, Inc., 59 Temple Place,          *
 *   Suite 330, Boston, MA  02111-1307, USA                                 *
 *                                                                          *
 ****************************************************************************/

#include "PreCompiled.h"

#ifndef _PreComp_
# include <cmath>
# include <cstdlib>
# include <sstream>
# include <QString>
# include <BRepLib.hxx>
# include <BSplCLib.hxx>
# include <Bnd_Box.hxx>
# include <BRep_Builder.hxx>
# include <BRep_Tool.hxx>
# include <BRepAdaptor_Curve.hxx>
# include <BRepAdaptor_CompCurve.hxx>
# include <BRepAdaptor_HCurve.hxx>
# include <BRepAdaptor_HCompCurve.hxx>
# include <BRepAdaptor_Surface.hxx>
# include <BRepAlgoAPI_Common.hxx>
# include <BRepAlgoAPI_Cut.hxx>
# include <BRepAlgoAPI_Fuse.hxx>
# include <BRepAlgo_Fuse.hxx>
# include <BRepAlgoAPI_Section.hxx>
# include <BRepBndLib.hxx>
# include <BRepBuilderAPI_FindPlane.hxx>
# include <BRepLib_FindSurface.hxx>
# include <BRepBuilderAPI_GTransform.hxx>
# include <BRepBuilderAPI_MakeEdge.hxx>
# include <BRepBuilderAPI_MakeFace.hxx>
# include <BRepBuilderAPI_MakePolygon.hxx>
# include <BRepBuilderAPI_MakeSolid.hxx>
# include <BRepBuilderAPI_MakeVertex.hxx>
# include <BRepBuilderAPI_MakeWire.hxx>
# include <BRepBuilderAPI_MakeShell.hxx>
# include <BRepBuilderAPI_NurbsConvert.hxx>
# include <BRepBuilderAPI_FaceError.hxx>
# include <BRepBuilderAPI_Copy.hxx>
# include <BRepBuilderAPI_Transform.hxx>
# include <BRepCheck_Analyzer.hxx>
# include <BRepCheck_ListIteratorOfListOfStatus.hxx>
# include <BRepCheck_Result.hxx>
# include <BRepClass_FaceClassifier.hxx>
# include <BRepFilletAPI_MakeFillet.hxx>
# include <BRepFilletAPI_MakeChamfer.hxx>
# include <BRepGProp.hxx>
# include <BRepMesh_IncrementalMesh.hxx>
# include <BRepMesh_Triangle.hxx>
# include <BRepMesh_Edge.hxx>
# include <BRepOffsetAPI_MakeThickSolid.hxx>
# include <BRepOffsetAPI_MakeOffsetShape.hxx>
# include <BRepOffsetAPI_MakeOffset.hxx>
# include <BRepOffsetAPI_MakePipe.hxx>
# include <BRepOffsetAPI_MakePipeShell.hxx>
# include <BRepOffsetAPI_Sewing.hxx>
# include <BRepOffsetAPI_ThruSections.hxx>
# include <BRepPrimAPI_MakePrism.hxx>
# include <BRepPrimAPI_MakeRevol.hxx>
# include <BRepFeat_MakePrism.hxx>
# include <BRepTools.hxx>
# include <BRepTools_ReShape.hxx>
# include <BRepTools_ShapeSet.hxx>
# include <BRepTools_WireExplorer.hxx>
# include <BRepFill_CompatibleWires.hxx>
# include <GCE2d_MakeSegment.hxx>
# include <GCPnts_AbscissaPoint.hxx>
# include <GCPnts_UniformAbscissa.hxx>
# include <Geom2d_Line.hxx>
# include <Geom2d_TrimmedCurve.hxx>
# include <GeomLProp_SLProps.hxx>
# include <GeomAPI_ProjectPointOnSurf.hxx>
# include <GeomFill_CorrectedFrenet.hxx>
# include <GeomFill_CurveAndTrihedron.hxx>
# include <GeomFill_EvolvedSection.hxx>
# include <GeomFill_Pipe.hxx>
# include <GeomFill_SectionLaw.hxx>
# include <GeomFill_Sweep.hxx>
# include <GeomLib.hxx>
# include <GProp_GProps.hxx>
# include <ShapeConstruct_Curve.hxx>
# include <Law_BSpFunc.hxx>
# include <Law_BSpline.hxx>
# include <Law_BSpFunc.hxx>
# include <Law_Constant.hxx>
# include <Law_Linear.hxx>
# include <Law_S.hxx>
# include <TopTools_HSequenceOfShape.hxx>
# include <Interface_Static.hxx>
# include <IGESControl_Controller.hxx>
# include <IGESControl_Writer.hxx>
# include <IGESControl_Reader.hxx>
# include <IGESData_GlobalSection.hxx>
# include <IGESData_IGESModel.hxx>
# include <STEPControl_Writer.hxx>
# include <STEPControl_Reader.hxx>
# include <TopTools_MapOfShape.hxx>
# include <TopoDS.hxx>
# include <TopoDS_Compound.hxx>
# include <TopoDS_Iterator.hxx>
# include <TopoDS_Solid.hxx>
# include <TopoDS_Vertex.hxx>
# include <TopExp.hxx>
# include <TopExp_Explorer.hxx>
# include <TopTools_ListIteratorOfListOfShape.hxx>
# include <Geom2d_Ellipse.hxx>
# include <Geom_BezierCurve.hxx>
# include <Geom_BezierSurface.hxx>
# include <Geom_BSplineCurve.hxx>
# include <Geom_BSplineSurface.hxx>
# include <Geom_SurfaceOfLinearExtrusion.hxx>
# include <Geom_SurfaceOfRevolution.hxx>
# include <Geom_Circle.hxx>
# include <Geom_ConicalSurface.hxx>
# include <Geom_CylindricalSurface.hxx>
# include <Geom_Ellipse.hxx>
# include <Geom_Hyperbola.hxx>
# include <Geom_Line.hxx>
# include <Geom_Parabola.hxx>
# include <Geom_Plane.hxx>
# include <Geom_CartesianPoint.hxx>
# include <Geom_SphericalSurface.hxx>
# include <Geom_ToroidalSurface.hxx>
# include <GeomLib_IsPlanarSurface.hxx>
# include <Poly_Triangulation.hxx>
# include <Standard_Failure.hxx>
# include <StlAPI_Writer.hxx>
# include <gp_GTrsf.hxx>
# include <ShapeAnalysis_Shell.hxx>
# include <ShapeBuild_ReShape.hxx>
# include <ShapeExtend_Explorer.hxx>
# include <ShapeFix_Edge.hxx>
# include <ShapeFix_Face.hxx>
# include <ShapeFix_Shell.hxx>
# include <ShapeFix_Solid.hxx>
# include <ShapeUpgrade_ShellSewing.hxx>
# include <ShapeUpgrade_RemoveInternalWires.hxx>
# include <Standard_Version.hxx>
# include <ShapeFix_Wire.hxx>
# include <ShapeAnalysis.hxx>
# include <BRepFill.hxx>
# include <BRepOffsetAPI_DraftAngle.hxx>
# include <BRepPrimAPI_MakeHalfSpace.hxx>
# include <gp_Pln.hxx>
# include <BRepLProp_SLProps.hxx>
# include <BRepGProp_Face.hxx>
#endif
# include <ShapeAnalysis_FreeBoundsProperties.hxx>
# include <ShapeAnalysis_FreeBoundData.hxx>
# include <ShapeAnalysis_FreeBounds.hxx>
# include <BRepOffsetAPI_MakeFilling.hxx>
# include <TopTools_DataMapIteratorOfDataMapOfShapeListOfShape.hxx>
# include <GeomFill_FillingStyle.hxx>
# include <GeomFill_BSplineCurves.hxx>
# include <BRepFill_Generator.hxx>

#include <array>
#include <deque>
#include <boost/algorithm/string/predicate.hpp>
#include <Base/Exception.h>
#include <Base/Console.h>


#include "PartPyCXX.h"
#include "TopoShape.h"
#include "TopoShapeOpCode.h"
#include "CrossSection.h"
#include "TopoShapeFacePy.h"
#include "TopoShapeEdgePy.h"
#include "TopoShapeVertexPy.h"
#include "modelRefine.h"
#include "Tools.h"
#include "FaceMaker.h"
#include "BRepOffsetAPI_MakeOffsetFix.h"
#include "Geometry.h"
#include "FaceMakerBullseye.h"

#define TOPOP_VERSION 15

FC_LOG_LEVEL_INIT("TopoShape",true,2);

using namespace Part;

#define _HANDLE_NULL_SHAPE(_msg,_throw) do {\
    if(_throw) {\
        FC_THROWM(NullShapeException,_msg);\
    }\
    FC_WARN(_msg);\
}while(0)

#define HANDLE_NULL_SHAPE _HANDLE_NULL_SHAPE("Null shape",true)
#define HANDLE_NULL_INPUT _HANDLE_NULL_SHAPE("Null input shape",true)
#define WARN_NULL_INPUT _HANDLE_NULL_SHAPE("Null input shape",false)

static void expandCompound(const TopoShape &shape, std::vector<TopoShape> &res) {
    if(shape.isNull())
        HANDLE_NULL_INPUT;
    if(shape.getShape().ShapeType() != TopAbs_COMPOUND) {
        res.push_back(shape);
        return;
    }
    for(auto &s : shape.getSubTopoShapes())
        expandCompound(s,res);
}

void ShapeMapper::expand(const TopoDS_Shape &d, std::vector<TopoDS_Shape> &shapes)
{
    if (d.IsNull()) return;
    for(TopExp_Explorer xp(d, TopAbs_FACE);xp.More();xp.Next())
        shapes.push_back(xp.Current());
    for(TopExp_Explorer xp(d, TopAbs_EDGE, TopAbs_FACE);xp.More();xp.Next())
        shapes.push_back(xp.Current());
    for(TopExp_Explorer xp(d, TopAbs_VERTEX, TopAbs_EDGE);xp.More();xp.Next())
        shapes.push_back(xp.Current());
}

void ShapeMapper::populate(bool generated,
                           const TopTools_ListOfShape &src,
                           const TopTools_ListOfShape &dst)
{
    for(TopTools_ListIteratorOfListOfShape it(src);it.More();it.Next())
        populate(generated, it.Value(), dst);
}

void ShapeMapper::populate(bool generated,
                           const TopoShape &src,
                           const TopTools_ListOfShape &dst)
{
    if(src.isNull())
        return;
    std::vector<TopoDS_Shape> dstShapes;
    for(TopTools_ListIteratorOfListOfShape it(dst);it.More();it.Next())
        expand(it.Value(), dstShapes);
    insert(generated, src.getShape(), dstShapes);
}

void ShapeMapper::insert(bool generated, const TopoDS_Shape &s, const TopoDS_Shape &d)
{
    if (s.IsNull() || d.IsNull()) return;
    // Prevent an element shape from being both generated and modified
    if (generated) {
        if (_modifiedShapes.count(d))
            return;
        _generatedShapes.insert(d);
    } else {
        if( _generatedShapes.count(d))
            return;
        _modifiedShapes.insert(d);
    }
    auto &entry = generated?_generated[s]:_modified[s];
    if(entry.shapeSet.insert(d).second)
        entry.shapes.push_back(d);
};

void ShapeMapper::insert(bool generated, const TopoDS_Shape &s, const std::vector<TopoDS_Shape> &d)
{
    if (s.IsNull() || d.empty()) return;
    auto &entry = generated?_generated[s]:_modified[s];
    for(auto &shape : d) {
        // Prevent an element shape from being both generated and modified
        if (generated) {
            if (_modifiedShapes.count(shape))
                continue;
            _generatedShapes.insert(shape);
        } else {
            if( _generatedShapes.count(shape))
                continue;
            _modifiedShapes.insert(shape);
        }
        if(entry.shapeSet.insert(shape).second)
            entry.shapes.push_back(shape);
    }
};

struct ShapeRelationKey {
    std::string name;
    bool sameType;
    ShapeRelationKey(const char *_name, bool sameType)
        :sameType(sameType)
    {
        if(!boost::starts_with(_name,TopoShape::elementMapPrefix()))
            name = _name;
        else
            name = _name + TopoShape::elementMapPrefix().size();
    }

    bool operator<(const ShapeRelationKey &other) const {
        if(sameType != other.sameType)
            return sameType;
        return name < other.name;
    }
};

class TopoShape::Cache {

public:
    TopoDS_Shape shape;
    TopLoc_Location loc;
    TopLoc_Location locInv;

    std::size_t memsize = 0;

    struct AncestorInfo {
        bool inited = false;
        TopTools_IndexedDataMapOfShapeListOfShape shapes;
    };
    class Info {
    private:
        Cache *owner = 0;
        TopTools_IndexedMapOfShape shapes;
        std::vector<TopoShape> topoShapes;
        std::array<AncestorInfo, TopAbs_SHAPE+1> ancestors;

        TopoShape _getTopoShape(const TopoShape &parent, int index) {
            TopoShape res;
            auto &s = topoShapes[index-1];
            if(parent.getShape().Location().IsIdentity()) {
                if(s.isNull()) {
                    s._Shape = shapes.FindKey(index);
                    s.Tag = parent.Tag;
                    s.mapSubElement(parent);
                }
                res = s;
            } else {
                if(s.isNull()) {
                    s._Shape = shapes.FindKey(index);
                    auto copy = parent;
                    // Subshapes are cached without any parent transformation,
                    // so we have to strip out the parent shape transformation
                    // before mapping.
                    copy.setShape(copy.getShape().Located(TopLoc_Location()),false);
                    s.Tag = copy.Tag;
                    s.mapSubElement(copy);
                }
                res = s;
                // Only apply the parent shape transformation to a copy of the
                // subshape
                res.setShape(res.getShape().Moved(parent.getShape().Location()),false);
            }
            res.Tag = parent.Tag;
            return res;
        }

    public:

        TopoShape getTopoShape(const TopoShape &parent, int index) {
            TopoShape res;
            if(index<=0 || index>shapes.Extent())
                return res;
            topoShapes.resize(shapes.Extent());
            return _getTopoShape(parent,index);
        }

        std::vector<TopoShape> getTopoShapes(const TopoShape &parent) {
            int count = shapes.Extent();
            std::vector<TopoShape> res;
            res.reserve(count);
            topoShapes.resize(count);
            for(int i=1;i<=count;++i)
                res.push_back(_getTopoShape(parent,i));
            return res;
        }

        TopoDS_Shape stripLocation(const TopoDS_Shape &parent, const TopoDS_Shape &child) {
            if(parent.Location() != owner->loc) {
                owner->loc = parent.Location();
                owner->locInv = parent.Location().Inverted();
            }
            return child.Located(owner->locInv*child.Location());
        }

        int find(const TopoDS_Shape &parent, const TopoDS_Shape &subshape) {
            if(parent.Location().IsIdentity())
                return shapes.FindIndex(subshape);
            return shapes.FindIndex(stripLocation(parent,subshape));
        }

        TopoDS_Shape find(const TopoDS_Shape &parent, int index) {
            if(index<=0 || index>shapes.Extent())
                return TopoDS_Shape();
            if(parent.Location().IsIdentity())
                return shapes.FindKey(index);
            else
                return shapes.FindKey(index).Moved(parent.Location());
        }

        int count() const {
            return shapes.Extent();
        }

        friend Cache;
    };

    std::array<Info,TopAbs_SHAPE+1> infos;
    std::map<ShapeRelationKey,std::vector<std::pair<std::string,std::string> > > relations;

    Cache(const TopoDS_Shape &s)
        :shape(s.Located(TopLoc_Location()))
    {}

    Info &getInfo(TopAbs_ShapeEnum type, bool clearTopoShapes=false) {
        auto &info = infos[type];
        if(!info.owner) {
            info.owner = this;
            if(!shape.IsNull()) {
                if(type == TopAbs_SHAPE) {
                    for(TopoDS_Iterator it(shape);it.More();it.Next())
                        info.shapes.Add(it.Value());
                }else
                    TopExp::MapShapes(shape, type, info.shapes);
            }
        }else if(clearTopoShapes)
            info.topoShapes.clear();
        return info;
    }

    int countShape(TopAbs_ShapeEnum type) {
        if(shape.IsNull())
            return 0;
        return getInfo(type).count();
    }

    int findShape(const TopoDS_Shape &parent, const TopoDS_Shape &subshape) {
        if(shape.IsNull() || subshape.IsNull())
            return 0;
        return getInfo(subshape.ShapeType()).find(parent,subshape);
    }

    TopoDS_Shape findShape(const TopoDS_Shape &parent, TopAbs_ShapeEnum type, int index) {
        if(!shape.IsNull())
            return getInfo(type).find(parent,index);
        return TopoDS_Shape();
    }

    TopoDS_Shape findAncestor(const TopoDS_Shape &parent, const TopoDS_Shape &subshape,
            TopAbs_ShapeEnum type, std::vector<TopoDS_Shape> *ancestors=0)
    {
        TopoDS_Shape ret;
        if(shape.IsNull() || subshape.IsNull() || type==TopAbs_SHAPE)
            return ret;

        auto &info = getInfo(type);

        auto &ainfo = info.ancestors[subshape.ShapeType()];
        if(!ainfo.inited) {
            ainfo.inited = true;
            TopExp::MapShapesAndAncestors(shape, subshape.ShapeType(), type, ainfo.shapes);
        }
        int index;
        if(parent.Location().IsIdentity())
            index = ainfo.shapes.FindIndex(subshape);
        else
            index = ainfo.shapes.FindIndex(info.stripLocation(parent,subshape));
        if(!index)
            return ret;
        const auto &shapes = ainfo.shapes.FindFromIndex(index);
        if(!shapes.Extent())
            return ret;

        if(ancestors) {
            ancestors->reserve(ancestors->size()+shapes.Extent());
            for(TopTools_ListIteratorOfListOfShape it(shapes);it.More();it.Next())
                ancestors->push_back(it.Value().Moved(parent.Location()));
        }
        return shapes.First().Moved(parent.Location());
    }

    std::size_t getMemSize();
};

void TopoShape::initCache(int reset, const char *file, int line) const{
    if(reset>0
            || !_Cache
            || !_Cache->shape.IsPartner(_Shape)
            || _Cache->shape.Orientation() != _Shape.Orientation())
    {
        if(_Cache && reset==0) {
            if(file)
                _FC_TRACE(file,line,"invalidate cache");
            else
                FC_TRACE("invalidate cache");
        }
        _Cache = std::make_shared<Cache>(_Shape);
    }
}

#define INIT_SHAPE_CACHE() initCache(0,__FILE__,__LINE__)

int TopoShape::findShape(const TopoDS_Shape &subshape) const {
    INIT_SHAPE_CACHE();
    return _Cache->findShape(_Shape,subshape);
}

TopoDS_Shape TopoShape::findShape(const char *name) const {
    if(!name)
        return TopoDS_Shape();
    if(boost::starts_with(name,elementMapPrefix()))
        name = getElementName(name);
    auto idx = shapeTypeAndIndex(name);
    if(!idx.second)
        return TopoDS_Shape();
    INIT_SHAPE_CACHE();
    return _Cache->findShape(_Shape,idx.first,idx.second);
}

TopoDS_Shape TopoShape::findShape(TopAbs_ShapeEnum type, int idx) const {
    INIT_SHAPE_CACHE();
    return _Cache->findShape(_Shape,type,idx);
}

std::vector<TopoShape> TopoShape::searchSubShape(
        const TopoShape &subshape, std::vector<std::string> *names,
        bool checkGeometry, double tol, double atol) const
{
    std::vector<TopoShape> res;
    if(subshape.isNull() || this->isNull())
        return res;
    double tol2 = tol*tol;
    int i=0;
    TopAbs_ShapeEnum shapeType = subshape.shapeType();
    switch(shapeType) {
    case TopAbs_VERTEX:
        // Vertex search will do comparison with tolerance to account for
        // rounding error inccured through transformation.
        for(auto &s : getSubTopoShapes(TopAbs_VERTEX)) {
            ++i;
            if(BRep_Tool::Pnt(TopoDS::Vertex(s.getShape())).SquareDistance(
                        BRep_Tool::Pnt(TopoDS::Vertex(subshape.getShape()))) <= tol2)
            {
                if(names)
                    names->push_back(std::string("Vertex")+std::to_string(i));
                res.push_back(s);
            }
        }
        break;
    case TopAbs_EDGE:
    case TopAbs_FACE: {
        std::unique_ptr<Geometry> g;
        bool isLine = false;
        bool isPlane = false;

        std::vector<TopoDS_Shape> vertices;
        TopoShape wire;
        if(shapeType == TopAbs_FACE) {
            wire = subshape.splitWires();
            vertices = wire.getSubShapes(TopAbs_VERTEX);
        } else
            vertices = subshape.getSubShapes(TopAbs_VERTEX);

        if(vertices.empty() || checkGeometry) {
            g = Geometry::fromShape(subshape.getShape());
            if(!g)
                return res;
            if (shapeType == TopAbs_EDGE)
                isLine = (g->isDerivedFrom(GeomLine::getClassTypeId())
                            || g->isDerivedFrom(GeomLineSegment::getClassTypeId()));
            else
                isPlane = g->isDerivedFrom(GeomPlane::getClassTypeId());
        }

        auto compareGeometry = [&](const TopoShape &s, bool strict) {
            std::unique_ptr<Geometry> g2(Geometry::fromShape(s.getShape()));
            if (!g2)
                return false;
            if (isLine && !strict) {
                // For lines, don't compare geometry, just check the
                // vertices below instead, because the exact same edge
                // may have different geometrical representation.
                if (!g2->isDerivedFrom(GeomLine::getClassTypeId())
                        && !g2->isDerivedFrom(GeomLineSegment::getClassTypeId()))
                    return false;
            } else if (isPlane && !strict) {
                // For planes, don't compare geometry either, so that
                // we don't need to worry about orientation and so on.
                // Just check the edges.
                if (!g2->isDerivedFrom(GeomPlane::getClassTypeId()))
                    return false;
            } else if(!g2 || !g2->isSame(*g,tol,atol))
                return false;
            return true;
        };

        if(vertices.empty()) {
            // Probably an infinite shape, so we have to search by geometry
            int idx = 0;
            for (auto &s : getSubTopoShapes(shapeType)) {
                ++idx;
                if (!s.countSubShapes(TopAbs_VERTEX) && compareGeometry(s, true)) {
                    if(names)
                        names->push_back(shapeName(shapeType) + std::to_string(idx));
                    res.push_back(s);
                }
            }
            break;
        }

        // The basic idea of shape search is about the same for both edge and face.
        // * Search the first vertex, which is done with tolerance.
        // * Find the ancestor shape of the found vertex
        // * Compare each vertex of the ancestor shape and the input shape
        // * Perform geometry comparison of the ancestor and input shape.
        //      * For face, perform addition geometry comparison of each edges.
        std::unordered_set<TopoShape> shapeSet;
        for(auto &v : searchSubShape(vertices[0],nullptr,checkGeometry,tol,atol)) {
            for(auto idx : findAncestors(v.getShape(), shapeType)) {
                auto s = getSubTopoShape(shapeType, idx);
                if(!shapeSet.insert(s).second)
                    continue;
                TopoShape otherWire;
                std::vector<TopoDS_Shape> otherVertices;
                if (shapeType == TopAbs_FACE) {
                    otherWire = s.splitWires();
                    if (wire.countSubShapes(TopAbs_EDGE) != otherWire.countSubShapes(TopAbs_EDGE))
                        continue;
                    otherVertices = otherWire.getSubShapes(TopAbs_VERTEX);
                } else
                    otherVertices = s.getSubShapes(TopAbs_VERTEX);
                if (otherVertices.size() != vertices.size())
                    continue;
                if(checkGeometry && !compareGeometry(s, false))
                    continue;
                unsigned i = 0;
                bool matched = true;
                for(auto &v : vertices) {
                    bool found = false;
                    for (unsigned j=0; j<otherVertices.size(); ++j) {
                        auto & v1 = otherVertices[i];
                        if (++i == otherVertices.size())
                            i = 0;
                        if(BRep_Tool::Pnt(TopoDS::Vertex(v)).SquareDistance(
                                    BRep_Tool::Pnt(TopoDS::Vertex(v1))) <= tol2)
                        {
                            found = true;
                            break;
                        }
                    }
                    if (!found) {
                        matched = false;
                        break;
                    }
                }
                if(!matched)
                    continue;

                if(shapeType == TopAbs_FACE && checkGeometry) {
                    // Is it really necessary to check geometries of each edge of a face?
                    // Right now we only do outer wire check
                    auto otherEdges = otherWire.getSubShapes(TopAbs_EDGE);
                    std::vector<std::unique_ptr<Geometry> > geos;
                    geos.resize(otherEdges.size());
                    bool matched = true;
                    unsigned i = 0;
                    auto edges = wire.getSubShapes(TopAbs_EDGE);
                    for(auto &e : edges) {
                        std::unique_ptr<Geometry> g(Geometry::fromShape(e));
                        if(!g) {
                            matched = false;
                            break;
                        }
                        bool isLine = false;
                        gp_Pnt pt1, pt2;
                        if (g->isDerivedFrom(GeomLine::getClassTypeId())
                                || g->isDerivedFrom(GeomLineSegment::getClassTypeId()))
                        {
                            pt1 = BRep_Tool::Pnt(TopExp::FirstVertex(TopoDS::Edge(e)));
                            pt2 = BRep_Tool::Pnt(TopExp::LastVertex(TopoDS::Edge(e)));
                            isLine = true;
                        }
                        // We will tolerate on edge reordering
                        bool found = false;
                        for (unsigned j=0; j<otherEdges.size(); j++) {
                            auto & e1 = otherEdges[i];
                            auto & g1 = geos[i];
                            if (++i >= otherEdges.size())
                                i = 0;
                            if (!g1) {
                                g1 = Geometry::fromShape(e1);
                                if (!g1)
                                    break;
                            }
                            if (isLine) {
                                if(g1->isDerivedFrom(GeomLine::getClassTypeId())
                                        || g1->isDerivedFrom(GeomLineSegment::getClassTypeId()))
                                {
                                    auto p1 = BRep_Tool::Pnt(TopExp::FirstVertex(TopoDS::Edge(e1)));
                                    auto p2 = BRep_Tool::Pnt(TopExp::LastVertex(TopoDS::Edge(e1)));
                                    if((p1.SquareDistance(pt1) <= tol2
                                                && p2.SquareDistance(pt2) <= tol2)
                                            || (p1.SquareDistance(pt2) <= tol2
                                                && p2.SquareDistance(pt1) <= tol2))
                                    {
                                        found = true;
                                        break;
                                    }
                                }
                                continue;
                            }

                            if(g1->isSame(*g,tol,atol)) {
                                found = true;
                                break;
                            }
                        }
                        if (!found) {
                            matched = false;
                            break;
                        }
                    }
                    if (!matched)
                        continue;
                }
                if(names)
                    names->push_back(shapeName(shapeType) + std::to_string(idx));
                res.push_back(s);
            }
        }
        break;
    }
    default:
        break;
    }
    return res;
}

int TopoShape::findAncestor(const TopoDS_Shape &subshape, TopAbs_ShapeEnum type) const {
    INIT_SHAPE_CACHE();
    return _Cache->findShape(_Shape,_Cache->findAncestor(_Shape,subshape,type));
}

TopoDS_Shape TopoShape::findAncestorShape(const TopoDS_Shape &subshape, TopAbs_ShapeEnum type) const {
    INIT_SHAPE_CACHE();
    return _Cache->findAncestor(_Shape,subshape,type);
}

std::vector<int> TopoShape::findAncestors(const TopoDS_Shape &subshape, TopAbs_ShapeEnum type) const {
    const auto &shapes = findAncestorsShapes(subshape,type);
    std::vector<int> ret;
    ret.reserve(shapes.size());
    for(const auto &shape : shapes)
        ret.push_back(findShape(shape));
    return ret;
}

std::vector<TopoDS_Shape> TopoShape::findAncestorsShapes(
        const TopoDS_Shape &subshape, TopAbs_ShapeEnum type) const
{
    INIT_SHAPE_CACHE();
    std::vector<TopoDS_Shape> shapes;
    _Cache->findAncestor(_Shape,subshape,type,&shapes);
    return shapes;
}

bool TopoShape::canMapElement(const TopoShape &other) const {
    if(isNull() || other.isNull())
        return false;
    if(!other.Tag && !other.getElementMapSize())
        return false;
    INIT_SHAPE_CACHE();
    other.INIT_SHAPE_CACHE();
    _Cache->relations.clear();
    return true;
}

void TopoShape::mapSubElement(const TopoShape &other, const char *op, bool forceHasher) {
    if(!canMapElement(other))
        return;

    bool warned = false;
    static const std::array<TopAbs_ShapeEnum,3> types = {TopAbs_VERTEX,TopAbs_EDGE,TopAbs_FACE};
    for(auto type : types) {
        auto &shapeMap = _Cache->getInfo(type,true);
        auto &otherMap = other._Cache->getInfo(type);
        if(!shapeMap.count() || !otherMap.count())
            continue;
        if(!forceHasher && other.Hasher) {
            if(Hasher) {
                if(other.Hasher!=Hasher) {
                    if(!getElementMapSize()) {
                        if(FC_LOG_INSTANCE.isEnabled(FC_LOGLEVEL_LOG))
                            FC_WARN("hasher mismatch");
                    }else {
                        // FC_THROWM(Base::RuntimeError, "hasher mismatch");
                        FC_ERR("hasher mismatch");
                    }
                    Hasher = other.Hasher;
                }
            }else
                Hasher = other.Hasher;
        }
        const char *shapetype = shapeName(type).c_str();
        std::ostringstream ss;

        bool forward;
        int count;
        if(otherMap.count()<shapeMap.count()) {
            forward = true;
            count = otherMap.count();
        }else{
            forward = false;
            count = shapeMap.count();
        }
        for(int k=1;k<=count;++k) {
            int i,idx;
            if(forward) {
                i = k;
                idx = shapeMap.find(_Shape,otherMap.find(other._Shape,k));
                if(!idx) continue;
            } else {
                idx = k;
                i = otherMap.find(other._Shape,shapeMap.find(_Shape,k));
                if(!i) continue;
            }
            ss.str("");
            ss << shapetype << idx;
            std::string element = ss.str();
            ss.str("");
            ss << shapetype << i;
            for(auto &v : other.getElementMappedNames(ss.str().c_str(),true)) {
                auto &name = v.first;
                auto &sids = v.second;
                if(sids.size()) {
                    if(!warned && !other.Hasher) {
                        warned = true;
                        FC_WARN("missing hasher");
                    }
                    if(Hasher != other.Hasher)
                        sids.clear();
                }
                ss.str("");
                encodeElementName(element[0],name,ss,sids,op,other.Tag);
                setElementName(element.c_str(),name.c_str(),ss.str().c_str(),&sids);
            }
        }
    }
}

std::vector<TopoDS_Shape> TopoShape::getSubShapes(TopAbs_ShapeEnum type, TopAbs_ShapeEnum avoid) const {
    std::vector<TopoDS_Shape> ret;
    if(isNull())
        return ret;
    if (avoid != TopAbs_SHAPE) {
        for (TopExp_Explorer exp(getShape(), type, avoid); exp.More(); exp.Next())
            ret.push_back(exp.Current());
        return ret;
    }
    INIT_SHAPE_CACHE();
    auto &info = _Cache->getInfo(type);
    int count = info.count();
    ret.reserve(count);
    for(int i=1;i<=count;++i)
        ret.push_back(info.find(_Shape,i));
    return ret;
}

std::vector<TopoShape> TopoShape::getSubTopoShapes(TopAbs_ShapeEnum type, TopAbs_ShapeEnum avoid) const {
    if(isNull())
        return std::vector<TopoShape>();
    INIT_SHAPE_CACHE();

    auto res = _Cache->getInfo(type).getTopoShapes(*this);
    if (avoid != TopAbs_SHAPE && hasSubShape(avoid)) {
        for (auto it = res.begin(); it != res.end(); ) {
            if (_Cache->findAncestor(_Shape, it->getShape(), avoid).IsNull())
                ++it;
            else
                it = res.erase(it);
        }
    }
    return res;
}

static const std::string _SubShape("SubShape");

std::pair<TopAbs_ShapeEnum,int> TopoShape::shapeTypeAndIndex(const char *name) {
    int idx = 0;
    auto type = shapeType(name,true);
    size_t len;
    if(type != TopAbs_SHAPE)
        len = shapeName(type).size();
    else {
        if(boost::starts_with(name,_SubShape))
            len = _SubShape.size();
        else
            len = 0;
    }

    if(len) {
        std::istringstream iss(name+len);
        iss >> idx;
        if(!iss.eof())
            idx = 0;
    }
    if(!idx)
        type = TopAbs_SHAPE;
    return std::make_pair(type,idx);
}

unsigned long TopoShape::countSubShapes(TopAbs_ShapeEnum type) const {
    INIT_SHAPE_CACHE();
    return _Cache->countShape(type);
}

unsigned long TopoShape::countSubShapes(const char* Type) const {
    auto type = shapeType(getElementName(Type),true);
    if(type == TopAbs_SHAPE) {
        if(Type && _SubShape == Type)
            return countSubShapes(type);
        return 0;
    }
    return countSubShapes(type);
}

bool TopoShape::hasSubShape(TopAbs_ShapeEnum type) const {
    return countSubShapes(type)!=0;
}

bool TopoShape::hasSubShape(const char *Type) const {
    auto idx = shapeTypeAndIndex(Type);
    return idx.second>0 && idx.second<=(int)countSubShapes(idx.first);
}

TopoShape TopoShape::getSubTopoShape(const char *Type, bool silent) const {
    Type = getElementName(Type);
    if(Type && boost::starts_with(Type,elementMapPrefix())) {
        if(!silent)
            FC_THROWM(Base::CADKernelError, "Mapped element not found: " << Type);
        return TopoShape();
    }

    auto res = shapeTypeAndIndex(Type);
    if(res.second<=0) {
        if(!silent)
            FC_THROWM(Base::CADKernelError,"Invalid shape name " << (Type?Type:""));
        return TopoShape();
    }
    return getSubTopoShape(res.first,res.second,silent);
}

TopoShape TopoShape::getSubTopoShape(TopAbs_ShapeEnum type, int idx, bool silent) const {
    if(isNull()) {
        if(!silent)
            FC_THROWM(NullShapeException,"null shape");
        return TopoShape();
    }
    if(idx <= 0) {
        if(!silent)
            FC_THROWM(Base::ValueError,"Invalid shape index " << idx);
        return TopoShape();
    }
    if(type<0 || type>TopAbs_SHAPE) {
        if(!silent)
            FC_THROWM(Base::ValueError,"Invalid shape type " << type);
        return TopoShape();
    }
    INIT_SHAPE_CACHE();
    auto &info = _Cache->getInfo(type);
    if(idx > info.count()) {
        if(!silent)
            FC_THROWM(Base::ValueError,"Shape index " << idx << " out of bound "  << info.count());
        return TopoShape();
    }

    return info.getTopoShape(*this,idx);
}

std::string TopoShape::getElementMapVersion() const{
    std::ostringstream ss;
    unsigned occ_ver;
    if((OCC_VERSION_HEX & 0xFF0000) == 0x070000)
        occ_ver = 0x070200;
    else
        occ_ver = OCC_VERSION_HEX;
    ss << TOPOP_VERSION << '.' << std::hex << occ_ver
        << '.' << Data::ComplexGeoData::getElementMapVersion();
    return ss.str();
}

TopoShape &TopoShape::makECompound(const std::vector<TopoShape> &shapes, const char *op, bool force)
{
    _Shape.Nullify();
    resetElementMap();

    if(!force && shapes.size()==1) {
        *this = shapes[0];
        return *this;
    }

    BRep_Builder builder;
    TopoDS_Compound comp;
    builder.MakeCompound(comp);

    if(shapes.empty()) {
        _Shape = comp;
        return *this;
    }

    int count = 0;
    for(auto &s : shapes) {
        if(s.isNull()) {
            WARN_NULL_INPUT;
            continue;
        }
        builder.Add(comp,s.getShape());
        ++count;
    }
    if(!count)
        HANDLE_NULL_SHAPE;
    _Shape = comp;
    mapSubElement(shapes,op);
    return *this;
}

bool TopoShape::_makETransform(const TopoShape &shape,
        const Base::Matrix4D &rclTrf, const char *op, bool checkScale, bool copy)
{
    if(checkScale) {
        if(rclTrf.hasScale()<0) {
            makEGTransform(shape,rclTrf,op,copy);
            return true;
        }
    }
    makETransform(shape,convert(rclTrf),op,copy);
    return false;
}

TopoShape &TopoShape::makETransform(const TopoShape &shape, const gp_Trsf &trsf, const char *op, bool copy) {
    resetElementMap();

    if(!copy) {
        // OCCT checks the ScaleFactor against gp::Resolution() which is DBL_MIN!!!
        copy = trsf.ScaleFactor()*trsf.HVectorialPart().Determinant() < 0. ||
               Abs(Abs(trsf.ScaleFactor()) - 1) > Precision::Confusion();
    }
    TopoShape tmp(shape);
    if(copy) {
        if(shape.isNull())
            HANDLE_NULL_INPUT;

        BRepBuilderAPI_Transform mkTrf(shape.getShape(), trsf, Standard_True);
        // TODO: calling Moved() is to make sure the shape has some Location,
        // which is necessary for STEP export to work. However, if we reach
        // here, it porabably means BRepBuilderAPI_Transform has modified
        // underlying shapes (because of scaling), it will break compound child
        // parent relationship anyway. In short, STEP import/export will most
        // likely break badly if there is any scaling involved
        tmp._Shape = mkTrf.Shape().Moved(gp_Trsf());
    }else
        tmp._Shape.Move(trsf);
    if(op || (shape.Tag && shape.Tag!=Tag)) {
        _Shape = tmp._Shape;
        tmp.initCache(1);
        mapSubElement(tmp,op);
    } else
        *this = tmp;
    return *this;
}

TopoShape &TopoShape::makEGTransform(const TopoShape &shape,
        const Base::Matrix4D &rclTrf, const char *op, bool copy)
{
    if(shape.isNull())
        HANDLE_NULL_INPUT;

    // if(!op) op = TOPOP_GTRANSFORM;
    gp_GTrsf mat;
    mat.SetValue(1,1,rclTrf[0][0]);
    mat.SetValue(2,1,rclTrf[1][0]);
    mat.SetValue(3,1,rclTrf[2][0]);
    mat.SetValue(1,2,rclTrf[0][1]);
    mat.SetValue(2,2,rclTrf[1][1]);
    mat.SetValue(3,2,rclTrf[2][1]);
    mat.SetValue(1,3,rclTrf[0][2]);
    mat.SetValue(2,3,rclTrf[1][2]);
    mat.SetValue(3,3,rclTrf[2][2]);
    mat.SetValue(1,4,rclTrf[0][3]);
    mat.SetValue(2,4,rclTrf[1][3]);
    mat.SetValue(3,4,rclTrf[2][3]);

    // geometric transformation
    BRepBuilderAPI_GTransform mkTrf(shape.getShape(), mat, copy);
    if (op)
        return makEShape(mkTrf,shape,op);

    // FIXME: confirm that topo index does not change with BRepBuilderAPI_GTransform
    _Shape = mkTrf.Shape();
    _ElementMap = shape._ElementMap;
    return *this;
}


TopoShape &TopoShape::makECopy(const TopoShape &shape, const char *op, bool copyGeom, bool copyMesh)
{
    _Shape.Nullify();
    resetElementMap();

    if(shape.isNull())
        return *this;

    TopoShape tmp(shape);
#if OCC_VERSION_HEX >= 0x070000
    tmp._Shape = BRepBuilderAPI_Copy(shape.getShape(),copyGeom,copyMesh).Shape();
#else
    tmp._Shape = BRepBuilderAPI_Copy(shape.getShape()).Shape();
#endif
    if(op || (shape.Tag && shape.Tag!=Tag)) {
        _Shape = tmp._Shape;
        mapSubElement(tmp,op);
    }else
        *this = tmp;
    return *this;
}

static std::vector<TopoShape> prepareProfiles(const std::vector<TopoShape> &shapes,size_t offset=0) {
    std::vector<TopoShape> ret;
    for(size_t i=offset;i<shapes.size();++i) {
        auto sh = shapes[i];
        if(sh.isNull())
            HANDLE_NULL_INPUT;
        auto shape = sh.getShape();
        // Allow compounds with a single face, wire or vertex or
        // if there are only edges building one wire
        if (shape.ShapeType() == TopAbs_COMPOUND) {
            sh = sh.makEWires();
            if(sh.isNull())
                HANDLE_NULL_INPUT;
            shape = sh.getShape();
        }
        if (shape.ShapeType() == TopAbs_FACE) {
            shape = sh.splitWires().getShape();
        } else if (shape.ShapeType() == TopAbs_WIRE) {
            BRepBuilderAPI_MakeWire mkWire(TopoDS::Wire(shape));
            shape = mkWire.Wire();
        } else if (shape.ShapeType() == TopAbs_EDGE) {
            BRepBuilderAPI_MakeWire mkWire(TopoDS::Edge(shape));
            shape = mkWire.Wire();
        } else if (shape.ShapeType() != TopAbs_VERTEX) {
            FC_THROWM(Base::CADKernelError,"Profile shape is not a vertex, edge, wire nor face.");
        }
        ret.push_back(shape);
    }
    if(ret.empty())
        FC_THROWM(Base::CADKernelError,"No profile");
    return ret;
}

TopoShape &TopoShape::makEPipeShell( const std::vector<TopoShape> &shapes,
        const Standard_Boolean make_solid, const Standard_Boolean isFrenet, int transition, const char *op)
{
    if(!op) op = TOPOP_PIPE_SHELL;
    _Shape.Nullify();
    resetElementMap();

    if(shapes.size()<2)
        FC_THROWM(Base::CADKernelError,"Not enough input shape");

    auto spine = shapes.front().makEWires();
    if(spine.isNull())
        HANDLE_NULL_INPUT;
    if(spine.getShape().ShapeType()!=TopAbs_WIRE)
        FC_THROWM(Base::CADKernelError,"Spine shape cannot form a single wire");

    BRepOffsetAPI_MakePipeShell mkPipeShell(TopoDS::Wire(spine.getShape()));
    BRepBuilderAPI_TransitionMode transMode;
    switch (transition) {
        case 1: transMode = BRepBuilderAPI_RightCorner;
            break;
        case 2: transMode = BRepBuilderAPI_RoundCorner;
            break;
        default: transMode = BRepBuilderAPI_Transformed;
            break;
    }
    mkPipeShell.SetMode(isFrenet);
    mkPipeShell.SetTransitionMode(transMode);

    for(auto &sh : prepareProfiles(shapes,1))
        mkPipeShell.Add(sh.getShape());

    if (!mkPipeShell.IsReady())
        FC_THROWM(Base::CADKernelError,"shape is not ready to build");
    else
        mkPipeShell.Build();

    if (make_solid)	mkPipeShell.MakeSolid();

    return makEShape(mkPipeShell,shapes,op);
}

TopoShape &TopoShape::makERuledSurface(const std::vector<TopoShape> &shapes,
        int orientation, const char *op)
{
    if(!op) op = TOPOP_RULED_SURFACE;
    _Shape.Nullify();
    resetElementMap();

    if(shapes.size()!=2)
        FC_THROWM(Base::CADKernelError,"Wrong number of input shape");

    std::array<TopoDS_Shape,2> curves;
    int i=0;
    for(auto &s : shapes) {
        if(s.isNull())
            HANDLE_NULL_INPUT;
        const auto &shape = s.getShape();
        auto type = shape.ShapeType();
        if(type == TopAbs_WIRE || type == TopAbs_EDGE) {
            curves[i++] = shape;
            continue;
        }
        auto count = s.countSubShapes(TopAbs_WIRE);
        if(count>1)
            FC_THROWM(Base::CADKernelError,"Input shape has more than one wire");
        if(count==1) {
            curves[i++] = s.getSubShape(TopAbs_WIRE,1);
            continue;
        }
        count = s.countSubShapes(TopAbs_EDGE);
        if(count==0)
            FC_THROWM(Base::CADKernelError,"Input shape has no edge");
        if(count == 1) {
            curves[i++] = s.getSubShape(TopAbs_EDGE,1);
            continue;
        }
        curves[i] = s.makEWires().getShape();
        if(curves[i].IsNull())
            HANDLE_NULL_INPUT;
        if(curves[i].ShapeType()!=TopAbs_WIRE)
            FC_THROWM(Base::CADKernelError,"Input shape forms more than one wire");
        ++i;
    }

    if(curves[0].ShapeType()!=curves[1].ShapeType()) {
        for(auto &curve : curves) {
            if(curve.ShapeType() == TopAbs_EDGE)
                curve = BRepLib_MakeWire(TopoDS::Edge(curve));
        }
    }

    auto &S1 = curves[0];
    auto &S2 = curves[1];
    bool isWire = S1.ShapeType()==TopAbs_WIRE;

    // https://forum.freecadweb.org/viewtopic.php?f=8&t=24052
    //
    // if both shapes are sub-elements of one common shape then the fill algorithm
    // leads to problems if the shape has set a placement
    // The workaround is to reset the placement before calling BRepFill and then
    // applying the placement to the output shape
    TopLoc_Location Loc = S1.Location();
    if(!Loc.IsIdentity() && Loc==S2.Location()) {
        S1.Location(TopLoc_Location());
        S2.Location(TopLoc_Location());
    }else
        Loc = TopLoc_Location();

    if (orientation == 0) {
        // Automatic
        Handle(Adaptor3d_HCurve) a1;
        Handle(Adaptor3d_HCurve) a2;
        if (!isWire) {
            BRepAdaptor_Curve adapt1(TopoDS::Edge(S1));
            BRepAdaptor_Curve adapt2(TopoDS::Edge(S2));
            a1 = new BRepAdaptor_HCurve(adapt1);
            a2 = new BRepAdaptor_HCurve(adapt2);
        }
        else {
            BRepAdaptor_CompCurve adapt1(TopoDS::Wire(S1));
            BRepAdaptor_CompCurve adapt2(TopoDS::Wire(S2));
            a1 = new BRepAdaptor_HCompCurve(adapt1);
            a2 = new BRepAdaptor_HCompCurve(adapt2);
        }

        if (!a1.IsNull() && !a2.IsNull()) {
            // get end points of 1st curve
            gp_Pnt p1 = a1->Value(a1->FirstParameter());
            gp_Pnt p2 = a1->Value(a1->LastParameter());
            if (S1.Orientation() == TopAbs_REVERSED) {
                std::swap(p1, p2);
            }

            // get end points of 2nd curve
            gp_Pnt p3 = a2->Value(a2->FirstParameter());
            gp_Pnt p4 = a2->Value(a2->LastParameter());
            if (S2.Orientation() == TopAbs_REVERSED) {
                std::swap(p3, p4);
            }

            // Form two triangles (P1,P2,P3) and (P4,P3,P2) and check their normals.
            // If the dot product is negative then it's assumed that the resulting face
            // is twisted, hence the 2nd edge is reversed.
            gp_Vec v1(p1, p2);
            gp_Vec v2(p1, p3);
            gp_Vec n1 = v1.Crossed(v2);

            gp_Vec v3(p4, p3);
            gp_Vec v4(p4, p2);
            gp_Vec n2 = v3.Crossed(v4);

            if (n1.Dot(n2) < 0) {
                S2.Reverse();
            }
        }
    }
    else if (orientation == 2) {
        // Reverse
        S2.Reverse();
    }

    TopoDS_Shape ruledShape;
    if (!isWire) {
        ruledShape = BRepFill::Face(TopoDS::Edge(S1), TopoDS::Edge(S2));
    }
    else {
        ruledShape = BRepFill::Shell(TopoDS::Wire(S1), TopoDS::Wire(S2));
    }

    // re-apply the placement in case we reset it
    if (!Loc.IsIdentity())
        ruledShape.Move(Loc);

    // Use empty mapper and let makEShape name the created surface with lower elements.
    return makESHAPE(ruledShape,Mapper(),shapes,op);
}

const std::vector<TopoDS_Shape> &
MapperMaker::modified(const TopoDS_Shape &s) const
{
    _res.clear();
    try {
        TopTools_ListIteratorOfListOfShape it;
        for (it.Initialize(maker.Modified(s)); it.More(); it.Next())
            _res.push_back(it.Value());
    } catch (const Standard_Failure & e) {
        if (FC_LOG_INSTANCE.isEnabled(FC_LOGLEVEL_LOG))
            FC_WARN("Exception on shape mapper: " << e.GetMessageString());
    }
    return _res;
}

const std::vector<TopoDS_Shape> &
MapperMaker::generated(const TopoDS_Shape &s) const
{
    _res.clear();
    try {
        TopTools_ListIteratorOfListOfShape it;
        for (it.Initialize(maker.Generated(s)); it.More(); it.Next())
            _res.push_back(it.Value());
    } catch (const Standard_Failure & e) {
        if (FC_LOG_INSTANCE.isEnabled(FC_LOGLEVEL_LOG))
            FC_WARN("Exception on shape mapper: " << e.GetMessageString());
    }
    return _res;
}

struct MapperThruSections: MapperMaker {
    TopoShape firstProfile;
    TopoShape lastProfile;

    MapperThruSections(BRepOffsetAPI_ThruSections &tmaker,
            const std::vector<TopoShape> &profiles)
        :MapperMaker(tmaker)
    {
        if(!tmaker.FirstShape().IsNull())
            firstProfile = profiles.front();
        if(!tmaker.LastShape().IsNull())
            lastProfile = profiles.back();
    }
    virtual const std::vector<TopoDS_Shape> &generated(const TopoDS_Shape &s) const override {
        MapperMaker::generated(s);
        if(_res.size()) return _res;
        try {
            auto &tmaker = static_cast<BRepOffsetAPI_ThruSections&>(maker);
            auto shape = tmaker.GeneratedFace(s);
            if(!shape.IsNull())
                _res.push_back(shape);
            if(firstProfile.getShape().IsSame(s) || firstProfile.findShape(s))
                _res.push_back(tmaker.FirstShape());
            else if(lastProfile.getShape().IsSame(s) || lastProfile.findShape(s))
                _res.push_back(tmaker.LastShape());
        } catch (const Standard_Failure & e) {
            if (FC_LOG_INSTANCE.isEnabled(FC_LOGLEVEL_LOG))
                FC_WARN("Exception on shape mapper: " << e.GetMessageString());
        }
        return _res;
    }
};

TopoShape &TopoShape::makEShape(BRepOffsetAPI_ThruSections &mk, const TopoShape &source,
        const char *op)
{
    if(!op) op = TOPOP_THRU_SECTIONS;
    return makEShape(mk,std::vector<TopoShape>(1,source),op);
}

TopoShape &TopoShape::makEShape(BRepOffsetAPI_ThruSections &mk, const std::vector<TopoShape> &sources,
        const char *op)
{
    if(!op) op = TOPOP_THRU_SECTIONS;
    return makESHAPE(mk.Shape(),MapperThruSections(mk,sources),sources,op);
}

TopoShape &TopoShape::makELoft(const std::vector<TopoShape> &shapes,
                               Standard_Boolean isSolid,
                               Standard_Boolean isRuled,
                               Standard_Boolean isClosed,
                               Standard_Integer maxDegree,
                               const char *op)
{
    if(!op) op = TOPOP_LOFT;
    _Shape.Nullify();
    resetElementMap();

    // http://opencascade.blogspot.com/2010/01/surface-modeling-part5.html
    BRepOffsetAPI_ThruSections aGenerator (isSolid,isRuled);
    aGenerator.SetMaxDegree(maxDegree);

    auto profiles = prepareProfiles(shapes);
    if (shapes.size() < 2)
        FC_THROWM(Base::CADKernelError,"Need at least two vertices, edges or wires to create loft face");

    for(auto &sh : profiles) {
        const auto &shape = sh.getShape();
        if(shape.ShapeType() == TopAbs_VERTEX)
            aGenerator.AddVertex(TopoDS::Vertex (shape));
        else
            aGenerator.AddWire(TopoDS::Wire (shape));
    }
    // close loft by duplicating initial profile as last profile.  not perfect.
    if (isClosed) {
    /* can only close loft in certain combinations of Vertex/Wire(Edge):
        - V1-W1-W2-W3-V2  ==> V1-W1-W2-W3-V2-V1  invalid closed
        - V1-W1-W2-W3     ==> V1-W1-W2-W3-V1     valid closed
        - W1-W2-W3-V1     ==> W1-W2-W3-V1-W1     invalid closed
        - W1-W2-W3        ==> W1-W2-W3-W1        valid closed*/
        if (profiles.back().getShape().ShapeType() == TopAbs_VERTEX) {
            Base::Console().Message("TopoShape::makeLoft: can't close Loft with Vertex as last profile. 'Closed' ignored.\n");
        }
        else {
            // repeat Add logic above for first profile
            const TopoDS_Shape& firstProfile = profiles.front().getShape();
            if (firstProfile.ShapeType() == TopAbs_VERTEX)  {
                aGenerator.AddVertex(TopoDS::Vertex (firstProfile));
            }
            else if (firstProfile.ShapeType() == TopAbs_EDGE)  {
                aGenerator.AddWire(BRepBuilderAPI_MakeWire(TopoDS::Edge(firstProfile)).Wire());
            }
            else if (firstProfile.ShapeType() == TopAbs_WIRE)  {
                aGenerator.AddWire(TopoDS::Wire (firstProfile));
            }
        }
    }

    Standard_Boolean anIsCheck = Standard_True;
    aGenerator.CheckCompatibility (anIsCheck);   // use BRepFill_CompatibleWires on profiles. force #edges, orientation, "origin" to match.

    aGenerator.Build();
    return makESHAPE(aGenerator.Shape(),MapperThruSections(aGenerator,profiles),shapes,op);
}

TopoShape &TopoShape::makEPrism(const TopoShape &base, const gp_Vec& vec, const char *op) {
    if(!op) op = TOPOP_EXTRUDE;
    _Shape.Nullify();
    resetElementMap();
    if(base.isNull())
        HANDLE_NULL_SHAPE;
    BRepPrimAPI_MakePrism mkPrism(base.getShape(), vec);
    return makEShape(mkPrism,base,op);
}

void GenericShapeMapper::init(const TopoShape &src, const TopoDS_Shape &dst)
{
    for (TopExp_Explorer exp(dst, TopAbs_FACE); exp.More(); exp.Next()) {
        const TopoDS_Shape &dstFace = exp.Current();
        if (src.findShape(dstFace))
            continue;

        std::unordered_map<TopoDS_Shape, int> map;
        bool found = false;

        // Try to find a face in the src that shares at least two edges (or one
        // closed edge) with dstFace.
        // TODO: consider degenerative cases of two or more edges on the same line.
        for (TopExp_Explorer it(dstFace, TopAbs_EDGE); it.More(); it.Next()) {
            int idx = src.findShape(it.Current());
            if (!idx)
                continue;
            TopoDS_Edge e = TopoDS::Edge(it.Current());
#if OCC_VERSION_HEX >= 0x070000
            if(BRep_Tool::IsClosed(e))
#else
            p1 = BRep_Tool::Pnt(TopExp::FirstVertex(e));
            p2 = BRep_Tool::Pnt(TopExp::LastVertex(e));
            if(p1.SquareDistance(p2)<1e-14)
#endif
            {
                // closed edge, one face is enough
                TopoDS_Shape face = src.findAncestorShape(
                        src.getSubShape(TopAbs_EDGE,idx), TopAbs_FACE);
                if (!face.IsNull()) {
                    this->insert(false, face, dstFace);
                    found = true;
                    break;
                }
                continue;
            }
            for (auto &face : src.findAncestorsShapes(src.getSubShape(TopAbs_EDGE,idx), TopAbs_FACE)) {
                int &cnt = map[face];
                if (++cnt == 2) {
                    this->insert(false, face, dstFace);
                    found = true;
                    break;
                }
                if (found)
                break;
            }
        }

        if (found) continue;

        // if no face matches, try search by geometry surface
        std::unique_ptr<Geometry> g(Geometry::fromShape(dstFace));
        if (!g) continue;

        for (auto &v : map) {
            std::unique_ptr<Geometry> g2(Geometry::fromShape(v.first));
            if (g2 && g2->isSame(*g,1e-7,1e-12)) {
                this->insert(false, v.first, dstFace);
                break;
            }
        }
    }
}

TopoShape &TopoShape::makEPrism(const TopoShape &base,
                                const TopoShape& profileshape,
                                const TopoShape& supportface,
                                const TopoShape& uptoface,
                                const gp_Dir& direction,
                                Standard_Integer Mode,
                                Standard_Boolean Modify,
                                const char *op)
{
    if(!op) op = TOPOP_PRISM;

    BRepFeat_MakePrism PrismMaker;

    TopoDS_Shape res = base.getShape();
    for (auto &face : profileshape.getSubTopoShapes(TopAbs_FACE)) {
        PrismMaker.Init(res, TopoDS::Face(face.getShape()),
                TopoDS::Face(supportface.getShape()), direction, Mode, Modify);

        PrismMaker.Perform(uptoface.getShape());

        if (!PrismMaker.IsDone() || PrismMaker.Shape().IsNull())
            FC_THROWM(Base::CADKernelError,"BRepFeat_MakePrism: Could not extrude the sketch!");

        res = PrismMaker.Shape();

        if (Mode == 2)
            Mode = 1;
    }
    // BRepFeat_MakePrism does not seem to have a working Generated() and
    // Modified() function, hence, the resulting element mapping will be
    // broken. We use own own mapper instead.

    GenericShapeMapper mapper;

    std::vector<TopoShape> srcShapes;
    srcShapes.push_back(base);
    if (!profileshape.isNull() && !base.findShape(profileshape.getShape()))
        srcShapes.push_back(profileshape);
    if (!supportface.isNull() && !base.findShape(supportface.getShape()))
        srcShapes.push_back(supportface);
    if (!uptoface.isNull() && !base.findShape(uptoface.getShape()))
        srcShapes.push_back(uptoface);

    TopoShape src;
    src = src.makECompound(srcShapes, nullptr, false);
    mapper.init(src, res);
    this->makESHAPE(res,mapper,{src},op);
    return *this;
}

TopoShape &TopoShape::makERevolve(const TopoShape &_base, const gp_Ax1& axis,
        double d, const char *face_maker, const char *op)
{
    if(!op) op = TOPOP_REVOLVE;
    _Shape.Nullify();
    resetElementMap();

    TopoShape base(_base);
    if(base.isNull())
        HANDLE_NULL_SHAPE;

    if(face_maker && !base.hasSubShape(TopAbs_FACE)) {
        if(!base.hasSubShape(TopAbs_WIRE))
            base = base.makEWires();
        base = base.makEFace(0,face_maker);
    }
    BRepPrimAPI_MakeRevol mkRevol(base.getShape(), axis,d);
    return makEShape(mkRevol,base,op);
}

TopoShape &TopoShape::makEMirror(const TopoShape &shape, const gp_Ax2 &ax2, const char *op) {
    if(!op) op = TOPOP_MIRROR;
    _Shape.Nullify();
    resetElementMap();

    if(shape.isNull())
        HANDLE_NULL_SHAPE;

    gp_Trsf mat;
    mat.SetMirror(ax2);
    TopLoc_Location loc = shape.getShape().Location();
    gp_Trsf placement = loc.Transformation();
    mat = placement * mat;
    BRepBuilderAPI_Transform mkTrf(shape.getShape(), mat);
    return makEShape(mkTrf,shape,op);
}

struct MapperSewing: Part::TopoShape::Mapper {
    BRepBuilderAPI_Sewing &maker;
    MapperSewing(BRepBuilderAPI_Sewing &maker)
        :maker(maker)
    {}
    virtual const std::vector<TopoDS_Shape> &modified(const TopoDS_Shape &s) const override {
        _res.clear();
        try {
            const auto &shape = maker.Modified(s);
            if(!shape.IsNull() && !shape.IsSame(s))
                _res.push_back(shape);
            else {
                const auto &sshape = maker.ModifiedSubShape(s);
                if(!sshape.IsNull() && !sshape.IsSame(s))
                    _res.push_back(sshape);
            }
        } catch (const Standard_Failure & e) {
            if (FC_LOG_INSTANCE.isEnabled(FC_LOGLEVEL_LOG))
                FC_WARN("Exception on shape mapper: " << e.GetMessageString());
        }
        return _res;
    }
};

TopoShape &TopoShape::makEShape(BRepBuilderAPI_Sewing &mk, const std::vector<TopoShape> &shapes,
        const char *op)
{
    if(!op) op = TOPOP_SEWING;
    return makESHAPE(mk.SewedShape(),MapperSewing(mk),shapes,op);
}

TopoShape &TopoShape::makEShape(BRepBuilderAPI_Sewing &mkShape,
            const TopoShape &source, const char *op)
{
    if(!op) op = TOPOP_SEWING;
    return makEShape(mkShape,std::vector<TopoShape>(1,source),op);
}

TopoShape &TopoShape::makEOffset(const TopoShape &shape,
        double offset, double tol, bool intersection, bool selfInter,
        short offsetMode, short join, bool fill, const char *op)
{
    if(!op) op = TOPOP_OFFSET;
    _Shape.Nullify();
    resetElementMap();

#if OCC_VERSION_HEX < 0x070200
    BRepOffsetAPI_MakeOffsetShape mkOffset(shape.getShape(), offset, tol, BRepOffset_Mode(offsetMode),
        intersection ? Standard_True : Standard_False,
        selfInter ? Standard_True : Standard_False,
        GeomAbs_JoinType(join));
#else
    BRepOffsetAPI_MakeOffsetShape mkOffset;
    mkOffset.PerformByJoin(shape.getShape(), offset, tol, BRepOffset_Mode(offsetMode),
                           intersection ? Standard_True : Standard_False,
                           selfInter ? Standard_True : Standard_False,
                           GeomAbs_JoinType(join));
#endif

    if (!mkOffset.IsDone())
        FC_THROWM(Base::CADKernelError,"BRepOffsetAPI_MakeOffsetShape not done");

    TopoShape res(Tag,Hasher);
    res.makEShape(mkOffset,shape,op);
    if(shape.hasSubShape(TopAbs_SOLID) && !res.hasSubShape(TopAbs_SOLID)) {
        try {
            res = res.makESolid();
        }catch (Standard_Failure &e) {
            FC_WARN("failed to make solid: " << e.GetMessageString());
        }
    }
    if (!fill) {
        *this = res;
        return *this;
    }

    //get perimeter wire of original shape.
    //Wires returned seem to have edges in connection order.
    ShapeAnalysis_FreeBoundsProperties freeCheck(shape.getShape());
    freeCheck.Perform();
    if (freeCheck.NbClosedFreeBounds() < 1)
    {
        FC_THROWM(Base::CADKernelError,"no closed bounds");
    }

    BRep_Builder builder;
    std::vector<TopoShape> shapes;
    for (int index = 1; index <= freeCheck.NbClosedFreeBounds(); ++index)
    {
        TopoShape originalWire(shape.Tag,shape.Hasher,freeCheck.ClosedFreeBound(index)->FreeBound());
        originalWire.mapSubElement(shape);
        const BRepAlgo_Image& img = mkOffset.MakeOffset().OffsetEdgesFromShapes();

        //build offset wire.
        TopoDS_Wire offsetWire;
        builder.MakeWire(offsetWire);
        for(const auto &s : originalWire.getSubShapes(TopAbs_EDGE)) {
            if (!img.HasImage(s))
            {
                FC_THROWM(Base::CADKernelError,"no image for shape");
            }
            const TopTools_ListOfShape& currentImage = img.Image(s);
            TopTools_ListIteratorOfListOfShape listIt;
            int edgeCount(0);
            TopoDS_Edge mappedEdge;
            for (listIt.Initialize(currentImage); listIt.More(); listIt.Next())
            {
                if (listIt.Value().ShapeType() != TopAbs_EDGE)
                    continue;
                edgeCount++;
                mappedEdge = TopoDS::Edge(listIt.Value());
            }

            if (edgeCount != 1)
            {
                std::ostringstream stream;
                stream << "wrong edge count: " << edgeCount << std::endl;
                FC_THROWM(Base::CADKernelError,stream.str().c_str());
            }
            builder.Add(offsetWire, mappedEdge);
        }
        std::vector<TopoShape> wires;
        wires.push_back(originalWire);
        wires.push_back(TopoShape(Tag,Hasher,offsetWire));
        wires.back().mapSubElement(res);

        //It would be nice if we could get thruSections to build planar faces
        //in all areas possible, so we could run through refine. I tried setting
        //ruled to standard_true, but that didn't have the desired affect.
        BRepOffsetAPI_ThruSections aGenerator;
        aGenerator.AddWire(TopoDS::Wire(originalWire.getShape()));
        aGenerator.AddWire(offsetWire);
        aGenerator.Build();
        if (!aGenerator.IsDone())
        {
            FC_THROWM(Base::CADKernelError,"ThruSections failed");
        }

        shapes.push_back(TopoShape(Tag,Hasher).makEShape(aGenerator,wires));
    }

    TopoShape perimeterCompound(Tag,Hasher);
    perimeterCompound.makECompound(shapes,op);

    //still had to sew. not using the passed in parameter for sew.
    //Sew has it's own default tolerance. Opinions?
    BRepBuilderAPI_Sewing sewTool;
    sewTool.Add(shape.getShape());
    sewTool.Add(perimeterCompound.getShape());
    sewTool.Add(res.getShape());
    sewTool.Perform(); //Perform Sewing

    TopoDS_Shape outputShape = sewTool.SewedShape();
    if ((outputShape.ShapeType() == TopAbs_SHELL) && (outputShape.Closed()))
    {
        BRepBuilderAPI_MakeSolid solidMaker(TopoDS::Shell(outputShape));
        if (solidMaker.IsDone())
        {
            TopoDS_Solid temp = solidMaker.Solid();
            //contrary to the occ docs the return value OrientCloseSolid doesn't
            //indicate whether the shell was open or not. It returns true with an
            //open shell and we end up with an invalid solid.
            if (BRepLib::OrientClosedSolid(temp))
                outputShape = temp;
        }
    }

    shapes.clear();
    shapes.push_back(shape);
    shapes.push_back(res);
    shapes.push_back(perimeterCompound);
    *this = TopoShape(Tag,Hasher).makESHAPE(outputShape,MapperSewing(sewTool),shapes,op);
    return *this;
}

TopoShape &TopoShape::makEOffsetFace(const TopoShape &shape,
                                     double offset,
                                     double innerOffset,
                                     short joinType,
                                     short innerJoinType,
                                     const char *op)
{
    if (std::abs(innerOffset) < Precision::Confusion()
            && std::abs(offset) < Precision::Confusion()) {
        *this = shape;
        return *this;
    }

    if (shape.isNull())
        FC_THROWM(Base::ValueError, "makeOffsetFace: input shape is null!");
    if (!shape.hasSubShape(TopAbs_FACE))
        FC_THROWM(Base::ValueError, "makeOffsetFace: no face found");

    std::vector<TopoShape> res;
    for (auto & face : shape.getSubTopoShapes(TopAbs_FACE)) {
        std::vector<TopoShape> wires;
        TopoShape outerWire = face.splitWires(&wires, ReorientForward);
        if (wires.empty()) {
            res.push_back(makEOffset2D(face, offset, joinType, false, false, false, op));
            continue;
        }

        if (outerWire.isNull())
            FC_THROWM(Base::CADKernelError, "makeOffsetFace: missing outer wire!");

        if (std::abs(offset) > Precision::Confusion())
            outerWire = outerWire.makEOffset2D(offset, joinType, false, false, false, op);

        if (std::abs(innerOffset) > Precision::Confusion()) {
            TopoShape innerWires(0, Hasher);
            innerWires.makECompound(wires, "", false);
            innerWires = innerWires.makEOffset2D(innerOffset, innerJoinType, false, false, true, op);
            wires = innerWires.getSubTopoShapes(TopAbs_WIRE);
        }
        wires.push_back(outerWire);
        gp_Pln pln;
        res.push_back(TopoShape(0, Hasher).makEFace(wires,
                                                    nullptr,
                                                    nullptr,
                                                    face.findPlane(pln) ? &pln : nullptr));
    }
    return makECompound(res, "", false);
}

TopoShape &TopoShape::makEOffset2D(const TopoShape &shape, double offset, short joinType,
        bool fill, bool allowOpenResult, bool intersection, const char *op)
{
    if(shape.isNull())
        FC_THROWM(Base::ValueError, "makeOffset2D: input shape is null!");
    if (allowOpenResult && OCC_VERSION_HEX < 0x060900)
        FC_THROWM(Base::AttributeError, "openResult argument is not supported on OCC < 6.9.0.");

    if(!op) op = TOPOP_OFFSET2D;
    _Shape.Nullify();
    resetElementMap();

    // OUTLINE OF MAKEOFFSET2D
    // * Prepare shapes to process
    // ** if _Shape is a compound, recursively call this routine for all subcompounds
    // ** if intrsection, dump all non-compound children into shapes to process; otherwise call this routine recursively for all children
    // ** if _shape isn't a compound, dump it straight to shapes to process
    // * Test for shape types, and convert them all to wires
    // * find plane
    // * OCC call (BRepBuilderAPI_MakeOffset)
    // * postprocessing (facemaking):
    // ** convert offset result back to faces, if inputs were faces
    // ** OR do offset filling:
    // *** for closed wires, simply feed source wires + offset wires to smart facemaker
    // *** for open wires, try to connect source anf offset result by creating new edges (incomplete implementation)
    // ** actual call to FaceMakerBullseye, unified for all facemaking.

    std::vector<TopoShape> shapesToProcess;
    std::vector<TopoShape> shapesToReturn;
    bool forceOutputCompound = false;

    if (shape.getShape().ShapeType() == TopAbs_COMPOUND){
        if (!intersection){
            //simply recursively process the children, independently
            expandCompound(shape,shapesToProcess);
            forceOutputCompound = true;
        } else {
            //collect non-compounds from this compound for collective offset. Process other shapes independently.
            for(auto &s : shape.getSubTopoShapes()) {
                if(s.getShape().ShapeType() == TopAbs_COMPOUND){
                    //recursively process subcompounds
                    shapesToReturn.push_back(TopoShape(Tag,Hasher).makEOffset2D(
                                s, offset, joinType, fill, allowOpenResult, intersection, op));
                    forceOutputCompound = true;
                } else {
                    shapesToProcess.push_back(s);
                }
            }
        }
    } else {
        shapesToProcess.push_back(shape);
    }

    if(shapesToProcess.size() > 0){
        TopoShape res(Tag,Hasher);

        //although 2d offset supports offsetting a face directly, it seems there is
        //no way to do a collective offset of multiple faces. So, we are doing it
        //by getting all wires from the faces, and applying offsets to them, and
        //reassembling the faces later.
        std::vector<TopoShape> sourceWires;
        bool haveWires = false;
        bool haveFaces = false;
        for(auto &s : shapesToProcess){
            const auto &sh = s.getShape();
            switch (sh.ShapeType()) {
                case TopAbs_EDGE:
                    sourceWires.push_back(s.makEWires());
                    haveWires = true;
                    break;
                case TopAbs_WIRE:
                    sourceWires.push_back(s);
                    haveWires = true;
                    break;
                case TopAbs_FACE:{
                    auto outerWire = s.splitWires(&sourceWires);
                    sourceWires.push_back(outerWire);
                    haveFaces = true;
                }break;
                default:
                    FC_THROWM(Base::TypeError, "makeOffset2D: input shape is not an edge, wire or face or compound of those.");
                break;
            }
        }
        if (haveWires && haveFaces)
            FC_THROWM(Base::TypeError, "makeOffset2D: collective offset of a mix of wires and faces is not supported");
        if (haveFaces)
            allowOpenResult = false;

        //find plane.
        gp_Pln workingPlane;
        if (!TopoShape().makECompound(sourceWires,"",false).findPlane(workingPlane))
            FC_THROWM(Base::CADKernelError,"makeOffset2D: wires are nonplanar or noncoplanar");

        //do the offset..
        TopoShape offsetShape;
        if (fabs(offset) > Precision::Confusion()){
            BRepOffsetAPI_MakeOffsetFix mkOffset(GeomAbs_JoinType(joinType), allowOpenResult);
            for(auto &w : sourceWires) {
                mkOffset.AddWire(TopoDS::Wire(w.getShape()));
            }
            try {
#if defined(__GNUC__) && defined (FC_OS_LINUX)
                Base::SignalException se;
#endif
                mkOffset.Perform(offset);
            }
            catch (Standard_Failure &){
                throw;
            }
            catch (...) {
                FC_THROWM(Base::CADKernelError,"BRepOffsetAPI_MakeOffset has crashed! (Unknown exception caught)");
            }
            if(mkOffset.Shape().IsNull())
                FC_THROWM(NullShapeException, "makeOffset2D: result of offsetting is null!");

            //Copying shape to fix strange orientation behavior, OCC7.0.0. See bug #2699
            // http://www.freecadweb.org/tracker/view.php?id=2699
            offsetShape = shape.makEShape(mkOffset,op).makECopy();

        } else {
            offsetShape = TopoShape(Tag,Hasher).makECompound(sourceWires,0,false);
        }

        std::vector<TopoShape> offsetWires;
        //interestingly, if wires are removed, empty compounds are returned by MakeOffset (as of OCC 7.0.0)
        //so, we just extract all nesting
        expandCompound(offsetShape,offsetWires);
        if (offsetWires.empty())
            FC_THROWM(Base::CADKernelError, "makeOffset2D: offset result has no wires.");

        std::vector<TopoShape> wiresForMakingFaces;
        if (!fill){
            if (haveFaces){
                wiresForMakingFaces.insert(wiresForMakingFaces.end(), offsetWires.begin(),offsetWires.end());
            } else {
                shapesToReturn.insert(shapesToReturn.end(),offsetWires.begin(),offsetWires.end());
            }
        } else {
            //fill offset
            if (fabs(offset) < Precision::Confusion())
                FC_THROWM(Base::ValueError, "makeOffset2D: offset distance is zero. Can't fill offset.");

            //filling offset. There are three major cases to consider:
            // 1. source wires and result wires are closed (simplest) -> make face
            // from source wire + offset wire
            //
            // 2. source wire is open, but offset wire is closed (if not
            // allowOpenResult). -> throw away source wire and make face right from
            // offset result.
            //
            // 3. source and offset wire are both open (note that there may be
            // closed islands in offset result) -> need connecting offset result to
            // source wire with new edges

            //first, lets split apart closed and open wires.
            std::vector<TopoShape> closedWires;
            std::vector<TopoShape> openWires;
            for(auto &w : sourceWires)
                if (BRep_Tool::IsClosed(TopoDS::Wire(w.getShape())))
                    closedWires.push_back(w);
                else
                    openWires.push_back(w);
            for(auto &w : offsetWires)
                if (BRep_Tool::IsClosed(TopoDS::Wire(w.getShape())))
                    closedWires.push_back(w);
                else
                    openWires.push_back(w);

            wiresForMakingFaces.insert(wiresForMakingFaces.end(),closedWires.begin(),closedWires.end());
            if (!allowOpenResult || openWires.size() == 0){
                //just ignore all open wires
            } else {
                //We need to connect open wires to form closed wires.

                //for now, only support offsetting one open wire -> there should be exactly two open wires for connecting
                if (openWires.size() != 2)
                    FC_THROWM(Base::CADKernelError, "makeOffset2D: collective offset with filling of multiple wires is not supported yet.");

                TopoShape openWire1 = openWires.front();
                TopoShape openWire2 = openWires.back();

                //find open vertices
                BRepTools_WireExplorer xp;
                xp.Init(TopoDS::Wire(openWire1.getShape()));
                TopoDS_Vertex v1 = xp.CurrentVertex();
                for(;xp.More();xp.Next()){};
                TopoDS_Vertex v2 = xp.CurrentVertex();

                //find open vertices
                xp.Init(TopoDS::Wire(openWire2.getShape()));
                TopoDS_Vertex v3 = xp.CurrentVertex();
                for(;xp.More();xp.Next()){};
                TopoDS_Vertex v4 = xp.CurrentVertex();

                //check
                if (v1.IsNull())  FC_THROWM(NullShapeException, "v1 is null");
                if (v2.IsNull())  FC_THROWM(NullShapeException, "v2 is null");
                if (v3.IsNull())  FC_THROWM(NullShapeException, "v3 is null");
                if (v4.IsNull())  FC_THROWM(NullShapeException, "v4 is null");

                //assemble new wire

                //we want the connection order to be
                //v1 -> openWire1 -> v2 -> (new edge) -> v4 -> openWire2(rev) -> v3 -> (new edge) -> v1
                //let's check if it's the case. If not, we reverse one wire and swap its endpoints.

                if (fabs(gp_Vec(BRep_Tool::Pnt(v2), BRep_Tool::Pnt(v3)).Magnitude() - fabs(offset)) <= BRep_Tool::Tolerance(v2) + BRep_Tool::Tolerance(v3)){
                    openWire2._Shape.Reverse();
                    std::swap(v3, v4);
                    v3.Reverse();
                    v4.Reverse();
                } else if ((fabs(gp_Vec(BRep_Tool::Pnt(v2), BRep_Tool::Pnt(v4)).Magnitude() - fabs(offset)) <= BRep_Tool::Tolerance(v2) + BRep_Tool::Tolerance(v4))){
                    //orientation is as expected, nothing to do
                } else {
                    FC_THROWM(Base::CADKernelError, "makeOffset2D: fill offset: failed to establish open vertex relationship.");
                }

                //now directions of open wires are aligned. Finally. make new wire!
                BRepBuilderAPI_MakeWire mkWire;
                //add openWire1
                BRepTools_WireExplorer it;
                for(it.Init(TopoDS::Wire(openWire1.getShape())); it.More(); it.Next()){
                    mkWire.Add(it.Current());
                }
                //add first joining edge
                mkWire.Add(BRepBuilderAPI_MakeEdge(v2,v4).Edge());
                //add openWire2, in reverse order
                openWire2._Shape.Reverse();
                for(it.Init(TopoDS::Wire(openWire2.getShape())); it.More(); it.Next()){
                    mkWire.Add(it.Current());
                }
                //add final joining edge
                mkWire.Add(BRepBuilderAPI_MakeEdge(v3,v1).Edge());

                mkWire.Build();

                wiresForMakingFaces.push_back(TopoShape(Tag,Hasher).makEShape(mkWire,openWires,op));
            }
        }

        //make faces
        if (wiresForMakingFaces.size()>0) {
            TopoShape face(0, Hasher);
            face.makEFace(wiresForMakingFaces, nullptr, nullptr, &workingPlane);
            expandCompound(face, shapesToReturn);
        }
    }

    return makECompound(shapesToReturn,op,forceOutputCompound);
}

TopoShape &TopoShape::makEThickSolid(const TopoShape &shape,
        const std::vector<TopoShape> &faces, double offset, double tol, bool intersection,
        bool selfInter, short offsetMode, short join, const char *op)
{
    if(!op) op = TOPOP_THICKEN;

    _Shape.Nullify();
    resetElementMap();

    if(shape.isNull())
        HANDLE_NULL_SHAPE;

    if(faces.empty())
        HANDLE_NULL_INPUT;

    if(fabs(offset) <= 2*tol) {
        *this = shape;
        return *this;
    }

    TopTools_ListOfShape remFace;
    for(auto &face : faces) {
        if(face.isNull())
            HANDLE_NULL_INPUT;
        if(!shape.findShape(face.getShape()))
            FC_THROWM(Base::CADKernelError,"face does not belong to the shape");
        remFace.Append(face.getShape());
    }
#if OCC_VERSION_HEX < 0x070200
    BRepOffsetAPI_MakeThickSolid mkThick(shape.getShape(), remFace, offset, tol, BRepOffset_Mode(offsetMode),
        intersection ? Standard_True : Standard_False,
        selfInter ? Standard_True : Standard_False,
        GeomAbs_JoinType(join));
#else
    BRepOffsetAPI_MakeThickSolid mkThick;
    mkThick.MakeThickSolidByJoin(shape.getShape(), remFace, offset, tol, BRepOffset_Mode(offsetMode),
        intersection ? Standard_True : Standard_False,
        selfInter ? Standard_True : Standard_False,
        GeomAbs_JoinType(join));
#endif
    return makEShape(mkThick,shape,op);
}

TopoShape &TopoShape::makEWires(const std::vector<TopoShape> &shapes, const char *op, bool fix, double tol) {
    if(shapes.empty())
        HANDLE_NULL_SHAPE;
    if(shapes.size() == 1)
        return makEWires(shapes[0],op);
    return makEWires(TopoShape(Tag).makECompound(shapes),op,fix,tol);
}

TopoShape &TopoShape::makEWires(const TopoShape &shape, const char *op, bool fix, double tol)
{
    _Shape.Nullify();
    resetElementMap();

    if(!op) op = TOPOP_WIRE;
    if(tol<Precision::Confusion()) tol = Precision::Confusion();

    // Can't user ShapeAnalysis_FreeBounds. It seems the output edges are
    // modified some how, and it is not obvious how to map the resulting edges.
#if 0

    Handle(TopTools_HSequenceOfShape) hEdges = new TopTools_HSequenceOfShape();
    Handle(TopTools_HSequenceOfShape) hWires = new TopTools_HSequenceOfShape();
    for(TopExp_Explorer xp(shape.getShape(),TopAbs_EDGE);xp.More();xp.Next())
        hEdges->Append(xp.Current());
    if(!hEdges->Length())
        HANDLE_NULL_SHAPE;
    ShapeAnalysis_FreeBounds::ConnectEdgesToWires(hEdges, tol, Standard_False, hWires);
    if(!hWires->Length())
        HANDLE_NULL_SHAPE;

    std::vector<TopoShape> wires;
    for (int i=1; i<=hWires->Length(); i++) {
        auto wire = hWires->Value(i);
        if(fix) {
            // Fix any topological issues of the wire
            ShapeFix_Wire aFix;
            aFix.SetPrecision(Precision::Confusion());
            aFix.Load(TopoDS::Wire(wire));
            aFix.FixReorder();
            aFix.FixConnected();
            aFix.FixClosed();
            wire = aFix.Wire();
        }
        wires.push_back(TopoShape(Tag,Hasher,wire));
    }
    shape.mapSubElementsTo(wires);
    return makECompound(wires,op,false);
#else
    (void)fix;
    std::vector<TopoShape> edges;
    std::list<TopoShape> edge_list;
    std::vector<TopoShape> wires;

    for(auto &e : shape.getSubTopoShapes(TopAbs_EDGE))
        edge_list.push_back(e);

    edges.reserve(edge_list.size());
    wires.reserve(edge_list.size());

    // sort them together to wires
    while (edge_list.size() > 0) {
        BRepBuilderAPI_MakeWire mkWire;
        // add and erase first edge
        edges.push_back(edge_list.front());
        edge_list.pop_front();
        mkWire.Add(TopoDS::Edge(edges.back().getShape()));
        edges.back().setShape(mkWire.Edge(),false);

        TopoDS_Wire new_wire = mkWire.Wire(); // current new wire

        // try to connect each edge to the wire, the wire is complete if no more edges are connectible
        bool found = false;
        do {
            found = false;
            for (auto it=edge_list.begin();it!=edge_list.end();++it) {
                mkWire.Add(TopoDS::Edge(it->getShape()));
                if (mkWire.Error() != BRepBuilderAPI_DisconnectedWire) {
                    // edge added ==> remove it from list
                    found = true;
                    edges.push_back(*it);
                    edges.back().setShape(mkWire.Edge(),false);
                    edge_list.erase(it);
                    new_wire = mkWire.Wire();
                    break;
                }
            }
        } while (found);

        // Fix any topological issues of the wire
        ShapeFix_Wire aFix;
        aFix.SetPrecision(tol);
        aFix.Load(new_wire);

        aFix.FixReorder();
        // Assuming FixReorder() just reorder and don't change the underlying
        // edges, we get the wire and do a name mapping now, as the following
        // two operations (FixConnected and FixClosed) may change the edges.
        wires.push_back(aFix.Wire());
        wires.back().mapSubElement(edges,op);

        aFix.FixConnected();
        aFix.FixClosed();
        // Now retrieve the shape and set it without touching element map
        wires.back().setShape(aFix.Wire(),false);
    }
    return makECompound(wires,0,false);
#endif
}

TopoShape &TopoShape::makEFace(const TopoShape &shape,
                               const char *op,
                               const char *maker,
                               const gp_Pln *pln)
{
    std::vector<TopoShape> shapes;
    if(shape.getShape().ShapeType() == TopAbs_COMPOUND)
        shapes = shape.getSubTopoShapes();
    else
        shapes.push_back(shape);
    return makEFace(shapes,op,maker,pln);
}

TopoShape &TopoShape::makEFace(const std::vector<TopoShape> &shapes,
                               const char *op,
                               const char *maker,
                               const gp_Pln *pln)
{
    _Shape.Nullify();
    resetElementMap();

    if(!maker || !maker[0]) maker = "Part::FaceMakerBullseye";
    std::unique_ptr<FaceMaker> mkFace = FaceMaker::ConstructFromType(maker);
    mkFace->MyHasher = Hasher;
    mkFace->MyOp = op;
    if (pln)
        mkFace->setPlane(*pln);

    for(auto &s : shapes) {
        if (s.getShape().ShapeType() == TopAbs_COMPOUND)
            mkFace->useTopoCompound(s);
        else
            mkFace->addTopoShape(s);
    }
    mkFace->Build();
    const auto &ret = mkFace->getTopoShape();
    _Shape = ret._Shape;
    _ElementMap = ret._ElementMap;
    Hasher = ret.Hasher;
    return *this;
}

class MyRefineMaker : public BRepBuilderAPI_RefineModel
{
public:
    MyRefineMaker(const TopoDS_Shape &s)
        :BRepBuilderAPI_RefineModel(s)
    {}

    void populate(ShapeMapper &mapper)
    {
        for (TopTools_DataMapIteratorOfDataMapOfShapeListOfShape it(this->myModified); it.More(); it.Next())
        {
            if (it.Key().IsNull()) continue;
            mapper.populate(false, it.Key(), it.Value());
        }
    }
};

TopoShape &TopoShape::makERefine(const TopoShape &shape, const char *op, bool no_fail) {
    _Shape.Nullify();
    resetElementMap();
    if(shape.isNull()) {
        if(!no_fail)
            HANDLE_NULL_SHAPE;
        return *this;
    }
    if(!op) op = TOPOP_REFINE;
    try {
#if 1
        MyRefineMaker mkRefine(shape.getShape());
        GenericShapeMapper mapper;
        mkRefine.populate(mapper);
        mapper.init(shape, mkRefine.Shape());
        return makESHAPE(mkRefine.Shape(), mapper, {shape}, op);
#else
        BRepBuilderAPI_RefineModel mkRefine(shape.getShape());
        return makEShape(mkRefine,shape,op);
#endif
    }catch (Standard_Failure &) {
        if(!no_fail) throw;
    }
    *this = shape;
    return *this;
}

TopoShape &TopoShape::makEShape(const char *maker,
        const TopoShape &shape, const char *op, double tol)
{
    return makEShape(maker,std::vector<TopoShape>(1,shape),op,tol);
}

// topo naming conterpart of TopoShape::makeShell()
TopoShape &TopoShape::makEShell(bool silent, const char *op) {
    if(silent) {
        if (isNull())
            return *this;

        if (shapeType(true) != TopAbs_COMPOUND)
            return *this;

        // we need a compound that consists of only faces
        TopExp_Explorer it;
        // no shells
        if (hasSubShape(TopAbs_SHELL))
            return *this;

        // no wires outside a face
        it.Init(_Shape, TopAbs_WIRE, TopAbs_FACE);
        if (it.More())
            return *this;

        // no edges outside a wire
        it.Init(_Shape, TopAbs_EDGE, TopAbs_WIRE);
        if (it.More())
            return *this;

        // no vertexes outside an edge
        it.Init(_Shape, TopAbs_VERTEX, TopAbs_EDGE);
        if (it.More())
            return *this;
    } else if (!hasSubShape(TopAbs_FACE)) {
        FC_THROWM(Base::CADKernelError,"Cannot make shell without face");
    }

    BRep_Builder builder;
    TopoDS_Shape shape;
    TopoDS_Shell shell;
    builder.MakeShell(shell);

    try {
        for (const auto &face : getSubShapes(TopAbs_FACE))
            builder.Add(shell, face);

        TopoShape tmp(Tag, Hasher, shell);
        tmp.mapSubElement(*this, op);

        shape = shell;
        BRepCheck_Analyzer check(shell);
        if (!check.IsValid()) {
            ShapeUpgrade_ShellSewing sewShell;
            shape = sewShell.ApplySewing(shell);
            // TODO confirm the above won't change OCCT topological naming
        }

        if (shape.IsNull()) {
            if (silent)
                return *this;
            FC_THROWM(NullShapeException, "Failed to make shell");
        }

        if (shape.ShapeType() != TopAbs_SHELL) {
            if (silent)
                return *this;
            FC_THROWM(Base::CADKernelError, "Failed to make shell: unexpected output shape type "
                    << shapeType(shape.ShapeType(), true));
        }

        _Shape = shape;
        _ElementMap = tmp._ElementMap;
    }
    catch (Standard_Failure &e) {
        if(!silent)
            FC_THROWM(Base::CADKernelError, "Failed to make shell: " << e.GetMessageString());
    }

    return *this;
}

struct MapperFill: Part::TopoShape::Mapper {
    BRepFill_Generator &maker;
    MapperFill(BRepFill_Generator &maker)
        :maker(maker)
    {}
    virtual const std::vector<TopoDS_Shape> &generated(const TopoDS_Shape &s) const override {
        _res.clear();
        try {
            TopTools_ListIteratorOfListOfShape it;
            for (it.Initialize(maker.GeneratedShapes(s)); it.More(); it.Next())
                _res.push_back(it.Value());
        } catch (const Standard_Failure & e) {
            if (FC_LOG_INSTANCE.isEnabled(FC_LOGLEVEL_LOG))
                FC_WARN("Exception on shape mapper: " << e.GetMessageString());
        }
        return _res;
    }
};

TopoShape &TopoShape::makEShellFromWires(const std::vector<TopoShape> &wires, bool silent, const char *op)
{
    BRepFill_Generator maker;
    for (auto &w : wires) {
        if (w.shapeType(silent) == TopAbs_WIRE)
            maker.AddWire(TopoDS::Wire(w.getShape()));
    }
    if (wires.empty()) {
        if (silent) {
            _Shape.Nullify();
            return *this;
        }
        FC_THROWM(NullShapeException,"No input shapes");
    }
    _Shape.Nullify();
    maker.Perform();
    this->makESHAPE(maker.Shell(), MapperFill(maker), wires, op);
    return *this;
}

TopoShape &TopoShape::makEShape(const char *maker,
        const std::vector<TopoShape> &shapes, const char *op, double tol)
{
#if OCC_VERSION_HEX <= 0x060800
    if (tol > 0.0)
        Standard_Failure::Raise("Fuzzy Booleans are not supported in this version of OCCT");
#endif

    if(!maker)
        FC_THROWM(Base::CADKernelError,"no maker");

    if(!op) op = maker;
    _Shape.Nullify();
    resetElementMap();

    if(shapes.empty())
        HANDLE_NULL_SHAPE;

    if(strcmp(maker,TOPOP_COMPOUND)==0) {
        return makECompound(shapes,op,false);
    } else if(boost::starts_with(maker,TOPOP_FACE)) {
        std::string prefix(TOPOP_FACE);
        prefix += '.';
        const char *face_maker = 0;
        if(boost::starts_with(maker,prefix))
            face_maker = maker+prefix.size();
        return makEFace(shapes,op,face_maker);
    } else if(strcmp(maker, TOPOP_WIRE)==0)
        return makEWires(shapes,op);
    else if(strcmp(maker, TOPOP_COMPSOLID)==0) {
        BRep_Builder builder;
        TopoDS_CompSolid Comp;
        builder.MakeCompSolid(Comp);
        for(auto &s : shapes) {
            if (!s.isNull())
                builder.Add(Comp, s.getShape());
        }
        _Shape = Comp;
        mapSubElement(shapes,op);
        return *this;
    }

    if(strcmp(maker,TOPOP_PIPE)==0) {
        if(shapes.size()!=2)
            FC_THROWM(Base::CADKernelError,"Not enough input shapes");
        if (shapes[0].isNull() || shapes[1].isNull())
            FC_THROWM(Base::CADKernelError,"Cannot sweep along empty spine");
        if (shapes[0].getShape().ShapeType() != TopAbs_WIRE)
            FC_THROWM(Base::CADKernelError,"Spine shape is not a wire");
        BRepOffsetAPI_MakePipe mkPipe(TopoDS::Wire(shapes[0].getShape()), shapes[1].getShape());
        return makEShape(mkPipe,shapes,op);
    }

    if(strcmp(maker,TOPOP_SHELL)==0) {
        BRep_Builder builder;
        TopoDS_Shell shell;
        builder.MakeShell(shell);
        for(auto &s : shapes)
            builder.Add(shell,s.getShape());
        _Shape = shell;
        mapSubElement(shapes,op);
        BRepCheck_Analyzer check(shell);
        if (!check.IsValid()) {
            ShapeUpgrade_ShellSewing sewShell;
            _Shape = sewShell.ApplySewing(shell);
            // TODO confirm the above won't change OCCT topological naming
        }
        return *this;
    }

    bool buildShell = true;

    std::vector<TopoShape> _shapes;
    if(strcmp(maker, TOPOP_FUSE)==0) {
        for(auto it=shapes.begin();it!=shapes.end();++it) {
            auto &s = *it;
            if(s.isNull())
                HANDLE_NULL_INPUT;
            if(s.shapeType() == TopAbs_COMPOUND) {
                if(_shapes.empty())
                    _shapes.insert(_shapes.end(),shapes.begin(),it);
                expandCompound(s,_shapes);
            }else if(_shapes.size())
                _shapes.push_back(s);
        }
    }
    else if (strcmp(maker, TOPOP_CUT)==0) {
        for(unsigned i=1; i<shapes.size(); ++i) {
            auto &s = shapes[i];
            if(s.isNull())
                HANDLE_NULL_INPUT;
            if(s.shapeType() == TopAbs_COMPOUND) {
                if(_shapes.empty())
                    _shapes.insert(_shapes.end(),shapes.begin(),shapes.begin()+i);
                expandCompound(s,_shapes);
            }else if(_shapes.size())
                _shapes.push_back(s);
        }
    }

    if (tol > 0.0 &&  _shapes.empty())
        _shapes = shapes;

    const auto &inputs = _shapes.size()?_shapes:shapes;
    if(inputs.empty())
        HANDLE_NULL_INPUT;
    if(inputs.size()==1) {
        *this = inputs[0];
        FC_WARN("Boolean operation with only one shape input");
        return *this;
    }

#if OCC_VERSION_HEX <= 0x060800
    TopoShape resShape = inputs[0];
    if (resShape.isNull())
        FC_THROWM(Base::ValueError,"Object shape is null");
    if(strcmp(maker, TOPOP_FUSE)==0) {
        for(size_t i=1;i<inputs.size();++i) {
            const auto &s = inputs[i];
            if (s.isNull())
                FC_THROWM(NullShapeException,"Input shape is null");
            BRepAlgoAPI_Fuse mk(resShape.getShape(), s.getShape());
            if (!mk.IsDone())
                FC_THROWM(Base::CADKernelError,"Fusion failed");
            resShape = makEShape(mk,{resShape,s},op);
        }
    } else if(strcmp(maker, TOPOP_CUT)==0) {
        for(size_t i=1;i<inputs.size();++i) {
            const auto &s = inputs[i];
            if (s.isNull())
                FC_THROWM(NullShapeException,"Input shape is null");
            BRepAlgoAPI_Cut mk(resShape.getShape(), s.getShape());
            if (!mk.IsDone())
                FC_THROWM(Base::CADKernelError,"Cut failed");
            resShape = makEShape(mk,{resShape,s},op);
        }
    } else if(strcmp(maker, TOPOP_COMMON)==0) {
        for(size_t i=1;i<inputs.size();++i) {
            const auto &s = inputs[i];
            if (s.isNull())
                FC_THROWM(NullShapeException,"Input shape is null");
            BRepAlgoAPI_Common mk(resShape.getShape(), s.getShape());
            if (!mk.IsDone())
                FC_THROWM(Base::CADKernelError,"Common failed");
            resShape = makEShape(mk,{resShape,s},op);
        }
    } else if(strcmp(maker, TOPOP_SECTION)==0) {
        buildShell = false;
        for(size_t i=1;i<inputs.size();++i) {
            const auto &s = inputs[i];
            if (s.isNull())
                FC_THROWM(NullShapeException,"Input shape is null");
            BRepAlgoAPI_Section mk(resShape.getShape(), s.getShape());
            if (!mk.IsDone())
                FC_THROWM(Base::CADKernelError,"Section failed");
            resShape = makEShape(mk,{resShape,s},op);
        }
    } else
        FC_THROWM(Base::CADKernelError,"Unknown maker");

    if(buildShell)
        makEShell();
    return *this;
#else

    std::unique_ptr<BRepAlgoAPI_BooleanOperation> mk;
    if(strcmp(maker, TOPOP_FUSE)==0)
        mk.reset(new BRepAlgoAPI_Fuse);
    else if(strcmp(maker, TOPOP_CUT)==0)
        mk.reset(new BRepAlgoAPI_Cut);
    else if(strcmp(maker, TOPOP_COMMON)==0)
        mk.reset(new BRepAlgoAPI_Common);
    else if(strcmp(maker, TOPOP_SECTION)==0) {
        mk.reset(new BRepAlgoAPI_Section);
        buildShell = false;
    } else
        FC_THROWM(Base::CADKernelError,"Unknown maker");

    TopTools_ListOfShape shapeArguments,shapeTools;

    int i=-1;
    for(const auto &shape : inputs) {
        if(shape.isNull())
            HANDLE_NULL_INPUT;
        if(++i == 0)
            shapeArguments.Append(shape.getShape());
        else if (tol > 0.0) {
            auto & s = _shapes[i];
            // workaround for http://dev.opencascade.org/index.php?q=node/1056#comment-520
            s.setShape(BRepBuilderAPI_Copy(s.getShape()).Shape(), false);
            shapeTools.Append(s.getShape());
        } else
            shapeTools.Append(shape.getShape());
    }

# if OCC_VERSION_HEX >= 0x060900
    if (shapeArguments.Size() + shapeTools.Size() > 2)
        mk->SetRunParallel(true);
# endif

    mk->SetArguments(shapeArguments);
    mk->SetTools(shapeTools);
    if (tol > 0.0) {
        mk->SetFuzzyValue(tol);
        mk->Build();
        makEShape(*mk,_shapes,op);
    } else {
        mk->Build();
        makEShape(*mk,shapes,op);
    }

    if(buildShell)
        makEShell();
    return *this;
#endif
}

TopoShape &TopoShape::makEShape(BRepBuilderAPI_MakeShape &mkShape,
        const TopoShape &source, const char *op)
{
    std::vector<TopoShape> sources;
    sources.emplace_back(source);
    return makEShape(mkShape,sources,op);
}

struct ShapeInfo {
    const TopoDS_Shape &shape;
    TopoShape::Cache::Info &cache;
    TopAbs_ShapeEnum type;
    const char *shapetype;

    ShapeInfo(const TopoDS_Shape &shape, TopAbs_ShapeEnum type, TopoShape::Cache::Info &cache)
        :shape(shape),cache(cache),type(type),shapetype(TopoShape::shapeName(type).c_str())
    {}

    int count() const {
        return cache.count();
    }

    TopoDS_Shape find(int index) {
        return cache.find(shape,index);
    }

    int find(const TopoDS_Shape &subshape) {
        return cache.find(shape,subshape);
    }
};

TopoShape &TopoShape::makEShape(BRepBuilderAPI_MakeShape &mkShape,
        const std::vector<TopoShape> &shapes, const char *op)
{
    return makESHAPE(mkShape.Shape(),MapperMaker(mkShape),shapes,op);
}

const char *TopoShape::setElementComboName(const char *element,
        const std::vector<std::string> &names, const char *marker, const char *op)
{
    if(names.empty())
        return 0;
    std::string _marker;
    if(!marker)
        marker = elementMapPrefix().c_str();
    else if(!boost::starts_with(marker,elementMapPrefix())){
        _marker = elementMapPrefix() + marker;
        marker = _marker.c_str();
    }
    auto it = names.begin();
    std::string newName = *it;
    std::ostringstream ss;
    std::vector<App::StringIDRef> sids;
    if(names.size() == 1)
        ss << marker;
    else {
        bool first = true;
        ss.str("");
        if(!Hasher)
            ss << marker;
        ss << '(';
        for(++it;it!=names.end();++it) {
            if(first)
                first = false;
            else
                ss << ',';
            ss << *it;
        }
        ss << ')';
        if(Hasher) {
            sids.push_back(Hasher->getID(ss.str().c_str()));
            ss.str("");
            ss << marker << sids.back()->toString();
        }
    }
    encodeElementName(element[0],newName,ss,sids,op);
    return setElementName(element,newName.c_str(),ss.str().c_str(),&sids);
}

struct NameKey {
    std::string name;
    long tag = 0;
    int shapetype = 0;

    NameKey()
    {}
    NameKey(const char *n)
        :name(n)
    {}
    NameKey(int type, const char *n)
        :name(n)
    {
        // Order the shape type from vertex < edge < face < other.  We'll rely
        // on this for sorting when we name the geometry element.
        switch(type) {
        case TopAbs_VERTEX:
            shapetype = 0;
            break;
        case TopAbs_EDGE:
            shapetype = 1;
            break;
        case TopAbs_FACE:
            shapetype = 2;
            break;
        default:
            shapetype = 3;
        }
    }
    bool operator<(const NameKey &other) const {
        if(shapetype < other.shapetype)
            return true;
        if(shapetype > other.shapetype)
            return false;
        if(tag < other.tag)
            return true;
        if(tag > other.tag)
            return false;
        return Data::ElementNameComp()(name,other.name);
    }
};

struct NameInfo {
    int index;
    std::vector<App::StringIDRef> sids;
    const char *shapetype;
};

TopoShape &TopoShape::makESHAPE(const TopoDS_Shape &shape, const Mapper &mapper,
        const std::vector<TopoShape> &shapes, const char *op)
{
    resetElementMap();
    _Shape = shape;
    if(shape.IsNull())
        HANDLE_NULL_SHAPE;

    if(shapes.empty())
        return *this;

    size_t canMap=0;
    for(auto &shape : shapes) {
        if(canMapElement(shape))
            ++canMap;
    }
    if(!canMap)
        return *this;
    if(canMap!=shapes.size() && FC_LOG_INSTANCE.isEnabled(FC_LOGLEVEL_LOG))
        FC_WARN("Not all input shapes are mappable");

    if(!op) op = TOPOP_MAKER;
    std::string _op = op;
    _op += '_';

    INIT_SHAPE_CACHE();
    ShapeInfo vinfo(_Shape,TopAbs_VERTEX,_Cache->getInfo(TopAbs_VERTEX));
    ShapeInfo einfo(_Shape,TopAbs_EDGE,_Cache->getInfo(TopAbs_EDGE));
    ShapeInfo finfo(_Shape,TopAbs_FACE,_Cache->getInfo(TopAbs_FACE));
    mapSubElement(shapes);

    std::array<ShapeInfo*,3> infos = {&vinfo,&einfo,&finfo};

    std::array<ShapeInfo*,TopAbs_SHAPE> infoMap;
    infoMap[TopAbs_VERTEX] = &vinfo;
    infoMap[TopAbs_EDGE] = &einfo;
    infoMap[TopAbs_WIRE] = &einfo;
    infoMap[TopAbs_FACE] = &finfo;
    infoMap[TopAbs_SHELL] = &finfo;
    infoMap[TopAbs_SOLID] = &finfo;
    infoMap[TopAbs_COMPOUND] = &finfo;
    infoMap[TopAbs_COMPSOLID] = &finfo;

    std::ostringstream ss;
    std::string postfix,newName;

    std::map<std::string,std::map<NameKey,NameInfo> > newNames;

    // First, collect names from other shapes that generates or modifies the
    // new shape
    for(auto &pinfo : infos) {
        auto &info = *pinfo;
        for(size_t n=0;n<shapes.size();++n) {
            const auto &other = shapes[n];
            if(!canMapElement(other))
                continue;
            auto &otherMap = other._Cache->getInfo(info.type);
            if(!otherMap.count())
                continue;

            for (int i=1; i<=otherMap.count(); i++) {
                const auto &otherElement = otherMap.find(other._Shape,i);
                // Find all new objects that are a modification of the old object
                ss.str("");
                ss << info.shapetype << i;
                std::vector<App::StringIDRef> sids;
                NameKey key(info.type, other.getElementName(ss.str().c_str(),MapToNamed,&sids));

                int k=0;
                for(auto &newShape : mapper.modified(otherElement)) {
                    ++k;
                    if(newShape.ShapeType()>=TopAbs_SHAPE) {
                        FC_ERR("unknown modified shape type " << newShape.ShapeType()
                                << " from " << info.shapetype << i);
                        continue;
                    }
                    auto &newInfo = *infoMap[newShape.ShapeType()];
                    if(newInfo.type != newShape.ShapeType()) {
                        FC_WARN("modified shape type " << shapeName(newShape.ShapeType())
                                << " mismatch with " << info.shapetype << i);
                        continue;
                    }
                    int j = newInfo.find(newShape);
                    if(!j) {
                        // This warning occurs in makERevolve. It generates
                        // some shape from a vertex that never made into the
                        // final shape. There may be other cases there.
                        if(FC_LOG_INSTANCE.isEnabled(FC_LOGLEVEL_LOG))
                            FC_WARN("Cannot find " << op << " modified " <<
                                newInfo.shapetype << " from " << info.shapetype << i);
                        continue;
                    }
                    ss.str("");
                    ss << newInfo.shapetype << j;
                    std::string element = ss.str();

                    if(getElementName(element.c_str(),MapToNamed)!=element.c_str())
                        continue;

                    key.tag = other.Tag;
                    auto &name_info = newNames[element][key];
                    name_info.sids = sids;
                    name_info.index = k;
                    name_info.shapetype = info.shapetype;
                }

                int checkParallel = -1;
                gp_Pln pln;

                // Find all new objects that were generated from an old object
                // (e.g. a face generated from an edge)
                k=0;
                for(auto &newShape : mapper.generated(otherElement)) {
                    if(newShape.ShapeType()>=TopAbs_SHAPE) {
                        FC_ERR("unknown generated shape type " << newShape.ShapeType()
                                << " from " << info.shapetype << i);
                        continue;
                    }

                    int parallelFace = -1;
                    int coplanarFace = -1;
                    auto &newInfo = *infoMap[newShape.ShapeType()];
                    std::vector<TopoDS_Shape> newShapes;
                    int shapeOffset = 0;
                    if(newInfo.type == newShape.ShapeType()) {
                        newShapes.push_back(newShape);
                    } else {
                        // It is possible for the maker to report generating a
                        // higher level shape, such as shell or solid. For
                        // example, when extruding, OCC will report the
                        // extruding face generating the entire solid. However,
                        // it will also report the edges of the extruding face
                        // generating the side faces. In this case, too much
                        // information is bad for us. We don't want the name of
                        // the side face (and its edges) to be coupled with
                        // other (unrelated) edges in the extruding face.
                        //
                        // shapeOffset below is used to make sure the higher
                        // level mapped names comes late after sorting. We'll
                        // ignore those names if there are more precise mapping
                        // available.
                        shapeOffset = 3;

                        if(info.type==TopAbs_FACE && checkParallel<0) {
                            if(!TopoShape(otherElement).findPlane(pln))
                                checkParallel = 0;
                            else
                                checkParallel = 1;
                        }
                        for(TopExp_Explorer xp(newShape,newInfo.type);xp.More();xp.Next()) {
                            newShapes.push_back(xp.Current());

                            if((parallelFace<0||coplanarFace<0) && checkParallel>0) {
                                // Specialized checking for high level mapped
                                // face that are either coplanar or parallel
                                // with the source face, which are common in
                                // operations like extrusion. Once found, the
                                // first coplanar face will assign an index of
                                // INT_MIN+1, and the first parallel face
                                // INT_MIN. The purpose of these special
                                // indexing is to make the name more stable for
                                // those generated faces.
                                //
                                // For example, the top or bottom face of an
                                // extrusion will be named using the extruding
                                // face. With a fixed index, the name is no
                                // longer affected by adding/removing of holes
                                // inside the extruding face/sketch.
                                gp_Pln plnOther;
                                if(TopoShape(newShapes.back()).findPlane(plnOther)) {
                                    if(pln.Axis().IsParallel(plnOther.Axis(),Precision::Angular())) {
                                        if(coplanarFace<0) {
                                            gp_Vec vec(pln.Axis().Location(),plnOther.Axis().Location());
                                            Standard_Real D1 = gp_Vec(pln.Axis().Direction()).Dot(vec);
                                            if (D1 < 0) D1 = - D1;
                                            Standard_Real D2 = gp_Vec(plnOther.Axis().Direction()).Dot(vec);
                                            if (D2 < 0) D2 = - D2;
                                            if(D1 <= Precision::Confusion() && D2 <= Precision::Confusion()) {
                                                coplanarFace = (int)newShapes.size();
                                                continue;
                                            }
                                        }
                                        if(parallelFace<0)
                                            parallelFace = (int)newShapes.size();
                                    }
                                }
                            }
                        }
                    }
                    key.shapetype += shapeOffset;
                    for(auto &newShape : newShapes) {
                        ++k;
                        int j = newInfo.find(newShape);
                        if(!j) {
                            if(FC_LOG_INSTANCE.isEnabled(FC_LOGLEVEL_LOG))
                                FC_WARN("Cannot find " << op << " generated " <<
                                        newInfo.shapetype << " from " << info.shapetype << i);
                            continue;
                        }
                        ss.str("");
                        ss << newInfo.shapetype << j;

                        std::string element = ss.str();
                        if(getElementName(element.c_str(),MapToNamed)!=element.c_str())
                            continue;

                        key.tag = other.Tag;
                        auto &name_info = newNames[element][key];
                        name_info.sids = sids;
                        if(k == parallelFace)
                            name_info.index = INT_MIN;
                        else if(k == coplanarFace)
                            name_info.index = INT_MIN+1;
                        else
                            name_info.index = -k;
                        name_info.shapetype = info.shapetype;
                    }
                    key.shapetype -= shapeOffset;
                }
            }
        }
    }

    // We shall first exclude those names generated from high level mapping. If
    // there are still any unnamed elements left after we go through the process
    // below, we set delayed=true, and start using those excluded names.
    bool delayed = false;

    while(true) {

        // Construct the names for modification/generation info collected in
        // the previous step
        for(auto itName=newNames.begin(),itNext=itName;
                itNext!=newNames.end();
                itName=itNext)
        {
            // We treat the first modified/generated source shape name specially.
            // If case there are more than one source shape. We hash the first
            // source name separately, and then obtain the second string id by
            // hashing all the source names together.  We then use the second
            // string id as the postfix for our name.
            //
            // In this way, we can associate the same source that are modified by
            // multiple other shapes.

            ++itNext;

            auto &element = itName->first;
            auto &names = itName->second;
            const auto &first_key = names.begin()->first;
            auto &first_info = names.begin()->second;

            if(!delayed && first_key.shapetype>=3 && first_info.index>INT_MIN+1) {
                // This name is mapped from high level (shell, solid, etc.)
                // Delay till next round.
                //
                // index>INT_MAX+1 is for checking generated coplanar and
                // parallel face mapping, which has special fixed index to make
                // name stable.  These names are not delayed.
                continue;
            }else if(!delayed && getElementName(element.c_str(),MapToNamed)!=element.c_str()) {
                newNames.erase(itName);
                continue;
            }

            int name_type = first_info.index>0?1:2; // index>0 means modified, or else generated
            std::string first_name = first_key.name;

            std::vector<App::StringIDRef> sids(first_info.sids);

            postfix.clear();
            if(names.size()>1) {
                ss.str("");
                ss << '(';
                bool first = true;
                auto it = names.begin();
                for(++it;it!=names.end();++it) {
                    auto &other_key = it->first;
                    if(other_key.shapetype>=3 && first_key.shapetype<3) {
                        // shapetype>=3 means its a high level mapping (e.g. a face
                        // generates a solid). We don't want that if there are more
                        // precise low level mapping available. See comments above
                        // for more details.
                        break;
                    }
                    if(first)
                        first = false;
                    else
                        ss << ',';
                    auto &other_info = it->second;
                    std::ostringstream ss2;
                    if(other_info.index!=1) {
                        // 'K' marks the additional source shape of this
                        // generate (or modified) shape.
                        ss2 << elementMapPrefix() << 'K';
                        if(other_info.index == INT_MIN)
                            ss2 << '0';
                        else if(other_info.index == INT_MIN+1)
                            ss2 << "00";
                        else {
                            // The same source shape may generate or modify
                            // more than one shape. The index here marks the
                            // position it is reported by OCC. Including the
                            // index here is likely to degrade name stablilty,
                            // but is unfortunately a necessity to avoid
                            // duplicate names.
                            ss2 << other_info.index;
                        }
                    }
                    std::string other_name = other_key.name;
                    encodeElementName(other_info.shapetype[0],other_name,ss2,sids,0,other_key.tag);
                    ss << other_name << ss2.str();
                    if((name_type==1 && other_info.index<0)
                            || (name_type==2 && other_info.index>0))
                    {
                        if (FC_LOG_INSTANCE.isEnabled(FC_LOGLEVEL_LOG))
                            FC_WARN("element is both generated and modified");
                        name_type = 0;
                    }
                    sids.insert(sids.end(),other_info.sids.begin(),other_info.sids.end());
                }
                if(!first) {
                    ss <<')';
                    if(Hasher) {
                        sids.push_back(Hasher->getID(ss.str().c_str()));
                        ss.str("");
                        ss << sids.back()->toString();
                    }
                    postfix = ss.str();
                }
            }

            ss.str("");
            if(name_type==2)
                ss << genPostfix();
            else if(name_type==1)
                ss << modPostfix();
            else
                ss << modgenPostfix();
            if(first_info.index == INT_MIN)
                ss << '0';
            else if(first_info.index == INT_MIN+1)
                ss << "00";
            else if(abs(first_info.index)>1)
                ss << abs(first_info.index);
            ss << postfix;
            encodeElementName(element[0],first_name,ss,sids,op,first_key.tag);
            setElementName(element.c_str(),first_name.c_str(),ss.str().c_str(),&sids);

            if(!delayed && first_key.shapetype<3)
                newNames.erase(itName);
        }

        // The reverse pass. Starting from the highest level element, i.e.
        // Face, for any element that are named, assign names for its lower unnamed
        // elements. For example, if Edge1 is named E1, and its vertexes are not
        // named, then name them as E1;U1, E1;U2, etc.
        //
        // In order to make the name as stable as possible, we may assign multiple
        // names (which must be sorted, because we may use the first one to name
        // upper element in the final pass) to lower element if it appears in
        // multiple higher elements, e.g. same edge in multiple faces.

        for(size_t ifo=infos.size()-1;ifo!=0;--ifo) {
            std::map<std::string,std::map<std::string, NameInfo, Data::ElementNameComp> > names;
            auto &info = *infos[ifo];
            auto &next = *infos[ifo-1];
            int i = 1;
            auto it = newNames.end();
            if(delayed)
                it = newNames.upper_bound(info.shapetype);
            for(;;++i) {
                std::string element;
                if(!delayed) {
                    if(i>info.count())
                        break;
                    ss.str("");
                    ss << info.shapetype << i;
                    element = ss.str();
                    if(newNames.count(element))
                        continue;
                }else if(it==newNames.end() ||
                        !boost::starts_with(it->first,info.shapetype))
                    break;
                else {
                    element = it->first;
                    ++it;
                    i = shapeTypeAndIndex(element.c_str()).second;
                    if(i==0 || i>info.count())
                        continue;
                }
                std::vector<App::StringIDRef> sids;
                const char *mapped = getElementName(element.c_str(),MapToNamed,&sids);
                if(mapped == element.c_str())
                    continue;

                TopTools_IndexedMapOfShape submap;
                TopExp::MapShapes(info.find(i), next.type, submap);
                for(int j=1,n=1;j<=submap.Extent();++j) {
                    ss.str("");
                    int k = next.find(submap(j));
                    assert(k);
                    ss << next.shapetype << k;
                    std::string element = ss.str();
                    if(getElementName(element.c_str(),MapToNamed) != element.c_str())
                        continue;
                    auto &info = names[element][mapped];
                    info.index = n++;
                    info.sids = sids;
                }
            }
            // Assign the actual names
            for(auto &v : names) {
#ifndef FC_ELEMENT_MAP_ALL
                // Do we really want multiple names for an element in this case?
                // If not, we just pick the name in the first sorting order here.
                auto &name = *v.second.begin();
#else
                for(auto &name : v.second)
#endif
                {
                    auto &info = name.second;
                    auto &sids = info.sids;
                    newName = name.first;
                    ss.str("");
                    ss << upperPostfix();
                    if(info.index>1)
                        ss << info.index;
                    encodeElementName(v.first[0],newName,ss,sids,op);
                    setElementName(v.first.c_str(),newName.c_str(),ss.str().c_str(),&sids);
                }
            }
        }

        // The forward pass. For any elements that are not named, try construct its
        // name from the lower elements
        bool hasUnnamed = false;
        for(size_t ifo=1;ifo<infos.size();++ifo) {
            auto &info = *infos[ifo];
            auto &prev = *infos[ifo-1];
            for(int i=1;i<=info.count();++i) {
                ss.str("");
                ss << info.shapetype << i;
                std::string element = ss.str();
                const char *name = getElementName(element.c_str(),MapToNamed);
                if(name != element.c_str())
                    continue;

                std::vector<App::StringIDRef> sids;
                std::map<std::string,std::string,Data::ElementNameComp> names;
                TopExp_Explorer xp;
                if(info.type == TopAbs_FACE)
                    xp.Init(BRepTools::OuterWire(TopoDS::Face(info.find(i))),TopAbs_EDGE);
                else
                    xp.Init(info.find(i),prev.type);
                for(;xp.More();xp.Next()) {
                    int j = prev.find(xp.Current());
                    assert(j);
                    ss.str("");
                    ss << prev.shapetype << j;
                    std::string element = ss.str();
                    if(!delayed && newNames.count(element)) {
                        names.clear();
                        break;
                    }
                    std::vector<App::StringIDRef> sid;
                    name = getElementName(element.c_str(),MapToNamed,&sid);
                    if(name == element.c_str()) {
                        // only assign name if all lower elements are named
                        if(FC_LOG_INSTANCE.isEnabled(FC_LOGLEVEL_LOG))
                            FC_WARN("unnamed lower element " << element);
                        names.clear();
                        break;
                    }
                    auto res = names.emplace(name,element);
                    if(res.second)
                        sids.insert(sids.end(),sid.begin(),sid.end());
                    else if(element!=res.first->second) {
                        // The seam edge will appear twice, which is normal. We
                        // only warn if the mapped element names are different.
                        FC_WARN("lower element " << element << " and " <<
                                res.first->second << " has duplicated name " << name
                                << " for " << info.shapetype << i );
                    }
                }
                if(names.empty()) {
                    hasUnnamed = true;
                    continue;
                }
                auto it = names.begin();
                newName = it->first;
                if(names.size() == 1)
                    ss << lowerPostfix();
                else {
                    bool first = true;
                    ss.str("");
                    if(!Hasher)
                        ss << lowerPostfix();
                    ss << '(';
                    for(++it;it!=names.end();++it) {
                        if(first)
                            first = false;
                        else
                            ss << ',';
                        ss << it->first;
                    }
                    ss << ')';
                    if(Hasher) {
                        sids.push_back(Hasher->getID(ss.str().c_str()));
                        ss.str("");
                        ss << lowerPostfix() << sids.back()->toString();
                    }
                }
                encodeElementName(element[0],newName,ss,sids,op);
                setElementName(element.c_str(),newName.c_str(),ss.str().c_str(),&sids);
            }
        }
        if(!hasUnnamed || delayed || newNames.empty())
            break;
        delayed = true;
    }
    return *this;
}

const std::string &TopoShape::modPostfix() {
    static std::string postfix(elementMapPrefix() + ":M");
    return postfix;
}

const std::string &TopoShape::modgenPostfix() {
    static std::string postfix(modPostfix() + "G");
    return postfix;
}

const std::string &TopoShape::genPostfix() {
    static std::string postfix(elementMapPrefix() + ":G");
    return postfix;
}

const std::string &TopoShape::upperPostfix() {
    static std::string postfix(elementMapPrefix() + ":U");
    return postfix;
}

const std::string &TopoShape::lowerPostfix() {
    static std::string postfix(elementMapPrefix() + ":L");
    return postfix;
}

TopoShape &TopoShape::makESlice(const TopoShape &shape,
        const Base::Vector3d& dir, double d, const char *op)
{
    _Shape.Nullify();
    resetElementMap();
    if(shape.isNull())
        HANDLE_NULL_SHAPE;
    TopoCrossSection cs(dir.x, dir.y, dir.z,shape,op);
    TopoShape res = cs.slice(1,d);
    resetElementMap(res._ElementMap);
    _Shape = res._Shape;
    Hasher = res.Hasher;
    return *this;
}

TopoShape &TopoShape::makESlice(const TopoShape &shape,
        const Base::Vector3d& dir, const std::vector<double> &d, const char *op)
{
    std::vector<TopoShape> wires;
    TopoCrossSection cs(dir.x, dir.y, dir.z, shape,op);
    int i=0;
    for(auto &dd : d)
        cs.slice(++i,dd,wires);
    return makECompound(wires,op,false);
}

TopoShape &TopoShape::makEFilledFace(const std::vector<TopoShape> &_shapes,
        const TopoShape &surface, const char *op)
{
    if(!op) op = TOPOP_FILLED_FACE;
    BRepOffsetAPI_MakeFilling maker;
    if (!surface.isNull() && surface.getShape().ShapeType() == TopAbs_FACE)
        maker.LoadInitSurface(TopoDS::Face(surface.getShape()));
    std::vector<TopoShape> shapes;
    for(auto &s : _shapes)
        expandCompound(s,shapes);
    int count = 0;
    for(auto &s : shapes) {
        if(s.isNull()) continue;
        const auto &sh = s.getShape();
        if (sh.ShapeType() == TopAbs_EDGE) {
            maker.Add(TopoDS::Edge(sh), GeomAbs_C0);
            ++count;
        }
        else if (sh.ShapeType() == TopAbs_FACE) {
            maker.Add(TopoDS::Face(sh), GeomAbs_C0);
            ++count;
        }
        else if (sh.ShapeType() == TopAbs_VERTEX) {
            maker.Add(BRep_Tool::Pnt(TopoDS::Vertex(sh)));
            ++count;
        }
    }
    if (!count)
        FC_THROWM(Base::CADKernelError,"Failed to created face with no constraints");
    maker.Build();
    if (!maker.IsDone())
        FC_THROWM(Base::CADKernelError,"Failed to created face by filling edges");
    return makEShape(maker,_shapes,op);
}

TopoShape &TopoShape::makESolid(const std::vector<TopoShape> &shapes, const char *op) {
    return makESolid(TopoShape().makECompound(shapes),op);
}

TopoShape &TopoShape::makESolid(const TopoShape &shape, const char *op) {
    _Shape.Nullify();
    if(!op) op = TOPOP_SOLID;
    resetElementMap();

    if(shape.isNull())
        HANDLE_NULL_SHAPE;

    //first, if we were given a compsolid, try making a solid out of it
    TopoDS_CompSolid compsolid;
    int count=0;
    for(const auto &s : shape.getSubShapes(TopAbs_COMPSOLID)) {
        ++count;
        compsolid = TopoDS::CompSolid(s);
        if (count > 1)
            break;
    }
    if (count == 0) {
        //no compsolids. Get shells...
        BRepBuilderAPI_MakeSolid mkSolid;
        count=0;
        for (const auto &s : shape.getSubShapes(TopAbs_SHELL)) {
            ++count;
            mkSolid.Add(TopoDS::Shell(s));
        }

        if (count == 0)//no shells?
            FC_THROWM(Base::CADKernelError,"No shells or compsolids found in shape");

        makEShape(mkSolid,shape,op);
        BRepLib::OrientClosedSolid(TopoDS::Solid(_Shape));
    } else if (count == 1) {
        BRepBuilderAPI_MakeSolid mkSolid(compsolid);
        makEShape(mkSolid,shape,op);
    } else { // if (count > 1)
        FC_THROWM(Base::CADKernelError,"Only one compsolid can be accepted. "
                "Provided shape has more than one compsolid.");
    }
    return *this;
}

TopoShape &TopoShape::replacEShape(const TopoShape &shape,
        const std::vector<std::pair<TopoShape,TopoShape> > &s)
{
    resetElementMap();
    _Shape.Nullify();
    if(shape.isNull())
        HANDLE_NULL_SHAPE;
    BRepTools_ReShape reshape;
    std::vector<TopoShape> shapes;
    shapes.reserve(s.size()+1);
    for (auto &v : s) {
        if(v.first.isNull() || v.second.isNull())
            HANDLE_NULL_INPUT;
        reshape.Replace(v.first.getShape(), v.second.getShape());
        shapes.push_back(v.second);
    }
    shapes.push_back(shape);
    _Shape = reshape.Apply(shape.getShape(),TopAbs_SHAPE);
    mapSubElement(shapes);
    return *this;
}

TopoShape &TopoShape::removEShape(const TopoShape &shape, const std::vector<TopoShape>& s)
{
    resetElementMap();
    _Shape.Nullify();
    if(shape.isNull())
        HANDLE_NULL_SHAPE;
    BRepTools_ReShape reshape;
    for(auto &sh : s) {
        if(sh.isNull())
            HANDLE_NULL_INPUT;
        reshape.Remove(sh.getShape());
    }
    _Shape = reshape.Apply(shape.getShape(), TopAbs_SHAPE);
    mapSubElement(shape);
    return *this;
}

TopoShape &TopoShape::makEFillet(const TopoShape &shape, const std::vector<TopoShape> &edges,
        double radius1, double radius2, const char *op)
{
    if(!op) op = TOPOP_FILLET;
    resetElementMap();
    _Shape.Nullify();
    if(shape.isNull())
        HANDLE_NULL_SHAPE;

    if(edges.empty())
        HANDLE_NULL_INPUT;

    BRepFilletAPI_MakeFillet mkFillet(shape.getShape());
    for(auto &e : edges) {
        if(e.isNull())
            HANDLE_NULL_INPUT;
        const auto &edge = e.getShape();
        if(!shape.findShape(edge))
            FC_THROWM(Base::CADKernelError,"edge does not belong to the shape");
        mkFillet.Add(radius1, radius2, TopoDS::Edge(edge));
    }
    return makEShape(mkFillet,shape,op);
}

TopoShape &TopoShape::makEChamfer(const TopoShape &shape, const std::vector<TopoShape> &edges,
        double radius1, double radius2, const char *op, bool flipDirection, bool asAngle)
{
    if(!op) op = TOPOP_CHAMFER;
    resetElementMap();
    _Shape.Nullify();
    if(shape.isNull())
        HANDLE_NULL_SHAPE;

    if(edges.empty())
        HANDLE_NULL_INPUT;

    BRepFilletAPI_MakeChamfer mkChamfer(shape.getShape());
    for(auto &e : edges) {
        const auto &edge = e.getShape();
        if(e.isNull())
            HANDLE_NULL_INPUT;
        if(!shape.findShape(edge))
            FC_THROWM(Base::CADKernelError,"edge does not belong to the shape");
        //Add edge to fillet algorithm
        TopoDS_Shape face;
        if(flipDirection)
            face = shape.findAncestorsShapes(edge,TopAbs_FACE).back();
        else
            face = shape.findAncestorShape(edge,TopAbs_FACE);
        if(asAngle)
            mkChamfer.AddDA(radius1, radius2, TopoDS::Edge(edge), TopoDS::Face(face));
        else
            mkChamfer.Add(radius1, radius2, TopoDS::Edge(edge), TopoDS::Face(face));
    }
    return makEShape(mkChamfer,shape,op);
}

TopoShape &TopoShape::makEGeneralFuse(const std::vector<TopoShape> &_shapes,
        std::vector<std::vector<TopoShape> > &modifies, double tol, const char *op)
{
#if OCC_VERSION_HEX < 0x060900
    (void)_shapes;
    (void)modifies;
    (void)tol;
    (void)op;
    FC_THROWM(Base::NotImplementedError,"GFA is available only in OCC 6.9.0 and up.");
#else
    if(!op) op = TOPOP_GENERAL_FUSE;
    resetElementMap();
    _Shape.Nullify();

    if(_shapes.empty())
        HANDLE_NULL_INPUT;

    std::vector<TopoShape> shapes(_shapes);

    BRepAlgoAPI_BuilderAlgo mkGFA;
    mkGFA.SetRunParallel(true);
    TopTools_ListOfShape GFAArguments;
    for(auto &shape : shapes) {
        if(shape.isNull())
            HANDLE_NULL_INPUT;
        if (tol > 0.0) {
            // workaround for http://dev.opencascade.org/index.php?q=node/1056#comment-520
            shape = shape.makECopy();
        }
        GFAArguments.Append(shape.getShape());
    }
    mkGFA.SetArguments(GFAArguments);
    if (tol > 0.0)
        mkGFA.SetFuzzyValue(tol);
#if OCC_VERSION_HEX >= 0x070000
    mkGFA.SetNonDestructive(Standard_True);
#endif
    mkGFA.Build();
    if (!mkGFA.IsDone())
        FC_THROWM(Base::CADKernelError,"GeneralFuse failed");
    makEShape(mkGFA,shapes,op);
    modifies.resize(shapes.size());
    int i=0;
    for(auto &s : shapes) {
        auto &mod = modifies[i++];
        for(TopTools_ListIteratorOfListOfShape it(mkGFA.Modified(s.getShape())); it.More(); it.Next()) {
            TopoShape res(Tag);
            res.setShape(it.Value());
            mod.push_back(res);
        }
        mapSubElementsTo(mod);
    }
    return *this;
#endif
}

TopoShape &TopoShape::makEFuse(const std::vector<TopoShape> &shapes,
        const char *op, double tol)
{
    return makEShape(TOPOP_FUSE,shapes,op,tol);
}

TopoShape &TopoShape::makECut(const std::vector<TopoShape> &shapes,
        const char *op, double tol)
{
    return makEShape(TOPOP_CUT,shapes,op,tol);
}


TopoShape &TopoShape::makEShape(BRepOffsetAPI_MakePipeShell &mkShape,
        const std::vector<TopoShape> &source, const char *op)
{
    if(!op) op = TOPOP_PIPE_SHELL;
    return makESHAPE(mkShape.Shape(),MapperMaker(mkShape),source,op);
}

TopoShape &TopoShape::makEShape(BRepFeat_MakePrism &mkShape,
        const TopoShape &source, const char *op)
{
    if(!op) op = TOPOP_PRISM;
    return makESHAPE(mkShape.Shape(),MapperMaker(mkShape),{source},op);
}

TopoShape &TopoShape::makEShape(BRepPrimAPI_MakeHalfSpace &mkShape,
        const TopoShape &source, const char *op)
{
    if(!op) op = TOPOP_HALF_SPACE;
    return makESHAPE(mkShape.Solid(),MapperMaker(mkShape),{source},op);
}

TopoShape &TopoShape::makEDraft(const TopoShape &shape, const std::vector<TopoShape> &_faces,
        const gp_Dir &pullDirection, double angle, const gp_Pln &neutralPlane,
        bool retry, const char *op)
{
    if(!op) op = TOPOP_DRAFT;

    resetElementMap();
    _Shape.Nullify();
    if(shape.isNull())
        HANDLE_NULL_SHAPE;

    std::vector<TopoShape> faces(_faces);
    bool done = true;
    BRepOffsetAPI_DraftAngle mkDraft;
    do {
        if(faces.empty())
            FC_THROWM(Base::CADKernelError,"no faces can be used");

        mkDraft.Init(shape.getShape());
        done = true;
        for(auto it=faces.begin();it!=faces.end();++it) {
            // TODO: What is the flag for?
            mkDraft.Add(TopoDS::Face(it->getShape()), pullDirection, angle, neutralPlane);
            if (!mkDraft.AddDone()) {
                // Note: the function ProblematicShape returns the face on which the error occurred
                // Note: mkDraft.Remove() stumbles on a bug in Draft_Modification::Remove() and is
                //       therefore unusable. See http://forum.freecadweb.org/viewtopic.php?f=10&t=3209&start=10#p25341
                //       The only solution is to discard mkDraft and start over without the current face
                // mkDraft.Remove(face);
                FC_ERR("Failed to add some face for drafting, skip");
                done = false;
                faces.erase(it);
                break;
            }
        }
    }while(retry && !done);

    mkDraft.Build();
    return makEShape(mkDraft,shape,op);
}

std::vector<std::pair<std::string,std::string> >
TopoShape::getRelatedElements(const char *_name, bool sameType) const {

    INIT_SHAPE_CACHE();
    ShapeRelationKey key(_name,sameType);
    auto it = _Cache->relations.find(key);
    if(it!=_Cache->relations.end())
        return it->second;

    std::vector<App::StringIDRef> sids;
    std::vector<std::pair<std::string,std::string> > ret;
    const std::string &name = key.name;
    long tag;
    size_t len;
    std::string postfix;
    char type;
    // extract tag and source element name length
    if(findTagInElementName(name,&tag,&len,&postfix,&type,true)==std::string::npos)
        return ret;

    // recover the original element name
    std::string original = name.substr(0,len);

    std::ostringstream ss;

    // First, search the name in the previous modeling step.
    auto dehashed = dehashElementName(original.c_str());
    long tag2;
    char type2;
    if(findTagInElementName(dehashed,&tag2,0,0,&type2,true)==std::string::npos) {
        ss.str("");
        encodeElementName(type,dehashed,ss,sids,0,tag);
        dehashed += ss.str();
    }else if(tag2!=tag && tag2!=-tag) {
        // Here means the dehashed element belongs to some other shape.
        // So just reset to original with middle markers stripped.
        dehashed = original + postfix;
    }
    auto element = getElementName(dehashed.c_str(),MapToIndexedForced);
    if(element!=dehashed.c_str() && (!sameType || type==element[0])) {
        ret.emplace_back(dehashed,element);
        _Cache->relations[key] = ret;
        return ret;
    }

    // Then, search any element that is modified from the given name
    ss.str("");
    ss << modPostfix();
    std::string modName(name);
    encodeElementName(type,modName,ss,sids);
    modName += ss.str();
    bool found = false;
    if(findTagInElementName(modName,&tag,&len,nullptr,nullptr,true)!=std::string::npos) {
        modName = modName.substr(0,len+modPostfix().size());
        for(auto &v : getElementNamesWithPrefix(modName.c_str())) {
            if((!sameType||type==v.second[0]) &&
               boost::ends_with(v.first,postfix))
            {
                found = true;
                ret.push_back(v);
            }
        }
    }

    // Finally, search any element that are modified from the same source of
    // the given name
    if(!found) {
        for(auto &v : getElementNamesWithPrefix((original+Part::TopoShape::modPostfix()).c_str())) {
            if((!sameType||type==v.second[0]) &&
               boost::ends_with(v.first,postfix))
                ret.push_back(v);
        }
    }
    _Cache->relations[key] = ret;
    return ret;
}

long TopoShape::isElementGenerated(const char *_name, int depth) const
{
    long res = 0;
    long tag = 0;
    traceElement(_name,
        [&] (const std::string &name, size_t offset, long tag2, long) {
            if(tag2 < 0)
                tag2 = -tag2;
            if(tag && tag2!=tag) {
                if(--depth < 1)
                    return true;
            }
            tag = tag2;
            if(depth==1 && boost::starts_with(name.c_str()+offset, genPostfix())) {
                res = tag;
                return true;
            }
            return false;
        });

    return res;
}

void TopoShape::reTagElementMap(long tag, App::StringHasherRef hasher, const char *postfix) {
    if(!tag) {
        FC_WARN("invalid shape tag for re-tagging");
        return;
    }

    if(_Shape.IsNull())
        return;

    TopoShape tmp(*this);
    initCache(1);
    Hasher = hasher;
    Tag = tag;
    resetElementMap();

    TopLoc_Location loc(tmp._Shape.Location());
    // Temporary reset shape location to make name mapping faster
    tmp._Shape.Location(TopLoc_Location());
    _Shape.Location(TopLoc_Location());
    mapSubElement(tmp,postfix,!hasher.isNull());
    _Shape.Location(loc);
}

void TopoShape::cacheRelatedElements(const char *name, bool sameType,
        const std::vector<std::pair<std::string,std::string> > &names) const
{
    INIT_SHAPE_CACHE();
    _Cache->relations[ShapeRelationKey(name,sameType)] = names;
}

bool TopoShape::getRelatedElementsCached(const char *name, bool sameType,
        std::vector<std::pair<std::string,std::string> > &names) const
{
    if(!_Cache)
        return false;
    auto it = _Cache->relations.find(ShapeRelationKey(name,sameType));
    if(it == _Cache->relations.end())
        return false;
    names.insert(names.end(),it->second.begin(),it->second.end());
    return true;
}

bool TopoShape::findPlane(gp_Pln &pln, double tol, double atol) const {
    if(_Shape.IsNull())
        return false;
    if (tol < 0.0)
        tol = Precision::Confusion();
    if (atol < 0.0)
        atol = Precision::Angular();
    TopoDS_Shape shape;
    if (countSubShapes(TopAbs_EDGE) == 1) {
        // To deal with OCCT bug of wrong edge transformation
        shape = BRepBuilderAPI_Copy(_Shape).Shape();
    } else
        shape = _Shape;
    try {
        bool found = false;
        // BRepLib_FindSurface only really works on edges. We'll deal face first
        for (auto &shape : getSubShapes(TopAbs_FACE)) {
            gp_Pln plane;
            auto face = TopoDS::Face(shape);
            BRepAdaptor_Surface adapt(face);
            if (adapt.GetType() == GeomAbs_Plane) {
                plane = adapt.Plane();
            } else {
                TopLoc_Location loc;
                Handle(Geom_Surface) surf = BRep_Tool::Surface(face, loc);
                GeomLib_IsPlanarSurface check(surf);
                if (check.IsPlanar())
                    plane = check.Plan();
                else
                    return false;
            }
            if (!found) {
                found = true;
                pln = plane;
            } else if (!pln.Position().IsCoplanar(plane.Position(), tol, atol))
                return false;
        }

        // Check if there is free edges (i.e. edges does not belong to any face)
        if (TopExp_Explorer(getShape(), TopAbs_EDGE, TopAbs_FACE).More()) {
            BRepLib_FindSurface finder(shape,tol,Standard_True);
            if (!finder.Found())
                return false;
            pln = GeomAdaptor_Surface(finder.Surface()).Plane();
            found = true;
        }

        // Check for free vertexes
        auto vertexes = getSubShapes(TopAbs_VERTEX, TopAbs_EDGE);
        if (vertexes.size()) {
            if (!found && vertexes.size() > 2) {
                BRep_Builder builder;
                TopoDS_Compound comp;
                builder.MakeCompound(comp);
                for (int i=0, c=(int)vertexes.size()-1; i<c; ++i) {
                    builder.Add(comp, 
                            BRepBuilderAPI_MakeEdge(TopoDS::Vertex(vertexes[i]),
                                                    TopoDS::Vertex(vertexes[i+1])).Edge());
                }
                BRepLib_FindSurface finder(comp,tol,Standard_True);
                if (!finder.Found())
                    return false;
                pln = GeomAdaptor_Surface(finder.Surface()).Plane();
                return true;
            }

            double tt = tol * tol;
            for (auto &v : vertexes) {
                if (pln.SquareDistance(BRep_Tool::Pnt(TopoDS::Vertex(v))) > tt)
                    return false;
            }
        }

        // To make the returned plane normal more stable, if the shape has any
        // face, use the normal of the first face.
        if (hasSubShape(TopAbs_FACE)) {
            shape = getSubShape(TopAbs_FACE, 1);
            BRepAdaptor_Surface adapt(TopoDS::Face(shape));
            double u = adapt.FirstUParameter()
                + (adapt.LastUParameter() - adapt.FirstUParameter())/2.;
            double v = adapt.FirstVParameter()
                + (adapt.LastVParameter() - adapt.FirstVParameter())/2.;
            BRepLProp_SLProps prop(adapt,u,v,2,Precision::Confusion());
            if(prop.IsNormalDefined()) {
                gp_Pnt pnt; gp_Vec vec;
                // handles the orientation state of the shape
                BRepGProp_Face(TopoDS::Face(shape)).Normal(u,v,pnt,vec);
                pln = gp_Pln(pnt, gp_Dir(vec));
            }
        }
        return true;
    }catch (Standard_Failure &e) {
        // For some reason the above BRepBuilderAPI_Copy failed to copy
        // the geometry of some edge, causing exception with message
        // BRepAdaptor_Curve::No geometry. However, without the above
        // copy, circular edges often have the wrong transformation!
        FC_LOG("failed to find surface: " << e.GetMessageString());
        return false;
    }
}

bool TopoShape::isCoplanar(const TopoShape &other, double tol, double atol) const {
    if(isNull() || other.isNull())
        return false;
    if(_Shape.IsEqual(other._Shape))
        return true;
    gp_Pln pln1,pln2;
    if(!findPlane(pln1,tol) || !other.findPlane(pln2,tol))
        return false;
    if(tol<0.0)
        tol = Precision::Confusion();
    if (atol<0.0)
        atol = Precision::Angular();
    return pln1.Position().IsCoplanar(pln2.Position(),tol,atol);
}

std::vector<std::string> TopoShape::getHigherElements(const char *element, bool silent) const
{
    TopoShape shape = getSubTopoShape(element, silent);
    if(shape.isNull())
        return {};

    std::vector<std::string> res;
    int type = shape.shapeType();
    for(;;) {
        if(--type < 0)
            break;
        for(int idx : findAncestors(shape.getShape(), (TopAbs_ShapeEnum)type)) {
            res.push_back(shapeName((TopAbs_ShapeEnum)type));
            res.back() += std::to_string(idx);
        }
    }
    return res;
}

bool TopoShape::isSame(const Data::ComplexGeoData &_other) const
{
    if(!_other.isDerivedFrom(TopoShape::getClassTypeId()))
        return false;

    const auto &other = static_cast<const TopoShape &>(_other);
    return Tag == other.Tag
        && Hasher == other.Hasher
        && _ElementMap == other._ElementMap
        && _Shape.IsEqual(other._Shape);
}

TopoShape & TopoShape::makEBSplineFace(const TopoShape & shape, FillingStyle style, const char *op)
{
    int edgeCount = shape.countSubShapes(TopAbs_EDGE);
    if (edgeCount < 2 || edgeCount > 4)
        FC_THROWM(Base::CADKernelError, "Require minimum two, maimum four edges");

    std::vector<Handle(Geom_BSplineCurve)> curves;
    curves.reserve(4);
    Standard_Real u1, u2; // contains output
    for (auto & e : shape.getSubTopoShapes(TopAbs_EDGE)) {
        const TopoDS_Edge& edge = TopoDS::Edge (e.getShape());
        TopLoc_Location heloc; // this will be output
        Handle(Geom_Curve) c_geom = BRep_Tool::Curve(edge, heloc, u1, u2); //The geometric curve
        Handle(Geom_BSplineCurve) bspline = Handle(Geom_BSplineCurve)::DownCast(c_geom); //Try to get BSpline curve
        if (!bspline.IsNull()) {
            gp_Trsf transf = heloc.Transformation();
            bspline->Transform(transf); // apply original transformation to control points
            //Store Underlying Geometry
            curves.push_back(bspline);
        }
        else {
            // try to convert it into a B-spline
            BRepBuilderAPI_NurbsConvert mkNurbs(edge);
            TopoDS_Edge nurbs = TopoDS::Edge(mkNurbs.Shape());
            // avoid copying
            TopLoc_Location heloc2; // this will be output
            Handle(Geom_Curve) c_geom2 = BRep_Tool::Curve(nurbs, heloc2, u1, u2); //The geometric curve
            Handle(Geom_BSplineCurve) bspline2 = Handle(Geom_BSplineCurve)::DownCast(c_geom2); //Try to get BSpline curve

            if (!bspline2.IsNull()) {
                gp_Trsf transf = heloc2.Transformation();
                bspline2->Transform(transf); // apply original transformation to control points
                //Store Underlying Geometry
                curves.push_back(bspline2);
            }
            else {
                // BRepBuilderAPI_NurbsConvert failed, try ShapeConstruct_Curve now
                ShapeConstruct_Curve scc;
                Handle(Geom_BSplineCurve) spline = scc.ConvertToBSpline(c_geom, u1, u2, Precision::Confusion());
                if (spline.IsNull())
                    Standard_Failure::Raise("A curve was not a B-spline and could not be converted into one.");
                gp_Trsf transf = heloc2.Transformation();
                spline->Transform(transf); // apply original transformation to control points
                curves.push_back(spline);
            }
        }
    }

    GeomFill_FillingStyle fstyle;
    switch (style) {
    case FillingStyle_Coons:
        fstyle = GeomFill_CoonsStyle;
        break;
    case FillingStyle_Curved:
        fstyle = GeomFill_CurvedStyle;
        break;
    default:
        fstyle = GeomFill_StretchStyle;
    }
    GeomFill_BSplineCurves aSurfBuilder; //Create Surface Builder

    if (edgeCount == 2) {
        aSurfBuilder.Init(curves[0], curves[1], fstyle);
    }
    else if (edgeCount == 3) {
        aSurfBuilder.Init(curves[0], curves[1], curves[2], fstyle);
    }
    else if (edgeCount == 4) {
        aSurfBuilder.Init(curves[0], curves[1], curves[2], curves[3], fstyle);
    }

    Handle(Geom_BSplineSurface) aSurface = aSurfBuilder.Surface();

    BRepBuilderAPI_MakeFace aFaceBuilder;
    Standard_Real v1, v2;
    // transfer surface bounds to face
    aSurface->Bounds(u1, u2, v1, v2);

    aFaceBuilder.Init(aSurface, u1, u2, v1, v2, Precision::Confusion());

    TopoShape aFace(0, Hasher, aFaceBuilder.Face());

    if (!aFaceBuilder.IsDone()) {
        FC_THROWM(Base::CADKernelError, "Face unable to be constructed");
    }
    if (aFace.isNull()) {
        FC_THROWM(Base::CADKernelError, "Resulting Face is null");
    }

    std::ostringstream ss;
    for (int i=0; i<edgeCount; ++i) {
        ss.str("");
        ss << "Edge" << (i+1);
        std::string element = ss.str();
        for(auto &v : getElementMappedNames(element.c_str(),true)) {
            auto &name = v.first;
            auto &sids = v.second;
            ss.str("");
            encodeElementName(element[0],name,ss,sids,op,this->Tag);
            aFace.setElementName(element.c_str(),name.c_str(),ss.str().c_str(),&sids);
        }
    }

    std::vector<std::string> names;
    names.push_back(aFace.getElementName("Edge1",MapToNamed));
    aFace.setElementComboName("Face1",names,op);
    *this = aFace;
    return *this;
}

static std::size_t
TopoShape_RefCountShapes(std::unordered_set<TopoDS_Shape> &shapeSet,
                         const TopoDS_Shape& aShape)
{
    std::size_t size = sizeof(Base::Placement); // rough estimate of location size
    if (aShape.IsNull())
        return size;

    // Recurse only for distinctive geometry shape. Other shapes are only
    // referenced with a location
    if (!shapeSet.insert(aShape.Located(TopLoc_Location())).second)
        return size;

    TopoDS_Iterator it;
    // go through all direct children
    for (it.Initialize(aShape, false, false);it.More(); it.Next()) {
        size += TopoShape_RefCountShapes(shapeSet, aShape);
    }

    return size;
}

std::size_t TopoShape::Cache::getMemSize()
{
    if (this->memsize || this->shape.IsNull())
        return this->memsize;

    std::unordered_set<TopoDS_Shape> shapeSet;
    this->memsize = TopoShape_RefCountShapes(shapeSet, this->shape);

    for (const auto & shape : shapeSet) {
        // Only check geometrical element for non compound shapes
        if (TopoDS_Iterator(shape, false, false).More())
            continue;

        // add the size of the underlying geomtric data
        Handle(TopoDS_TShape) tshape = shape.TShape();
        this->memsize += tshape->DynamicType()->Size();

        switch (shape.ShapeType())
        {
        case TopAbs_FACE:
            {
                // first, last, tolerance
                this->memsize += 5*sizeof(Standard_Real);
                const TopoDS_Face& face = TopoDS::Face(shape);
                // if no geometry is attached to a face an exception is raised
                BRepAdaptor_Surface surface;
                try {
                    surface.Initialize(face);
                }
                catch (const Standard_Failure&) {
                    continue;
                }
                switch (surface.GetType())
                {
                case GeomAbs_Plane:
                    this->memsize += sizeof(Geom_Plane);
                    break;
                case GeomAbs_Cylinder:
                    this->memsize += sizeof(Geom_CylindricalSurface);
                    break;
                case GeomAbs_Cone:
                    this->memsize += sizeof(Geom_ConicalSurface);
                    break;
                case GeomAbs_Sphere:
                    this->memsize += sizeof(Geom_SphericalSurface);
                    break;
                case GeomAbs_Torus:
                    this->memsize += sizeof(Geom_ToroidalSurface);
                    break;
                case GeomAbs_BezierSurface:
                    this->memsize += sizeof(Geom_BezierSurface);
                    this->memsize += (surface.NbUPoles()*surface.NbVPoles()) * sizeof(Standard_Real);
                    this->memsize += (surface.NbUPoles()*surface.NbVPoles()) * sizeof(Geom_CartesianPoint);
                    break;
                case GeomAbs_BSplineSurface:
                    this->memsize += sizeof(Geom_BSplineSurface);
                    this->memsize += (surface.NbUKnots()+surface.NbVKnots()) * sizeof(Standard_Real);
                    this->memsize += (surface.NbUPoles()*surface.NbVPoles()) * sizeof(Standard_Real);
                    this->memsize += (surface.NbUPoles()*surface.NbVPoles()) * sizeof(Geom_CartesianPoint);
                    break;
                case GeomAbs_SurfaceOfRevolution:
                    this->memsize += sizeof(Geom_SurfaceOfRevolution);
                    break;
                case GeomAbs_SurfaceOfExtrusion:
                    this->memsize += sizeof(Geom_SurfaceOfLinearExtrusion);
                    break;
                case GeomAbs_OtherSurface:
                    // What kind of surface should this be?
                    this->memsize += sizeof(Geom_Surface);
                    break;
                default:
                    break;
                }
            } break;
        case TopAbs_EDGE:
            {
                // first, last, tolerance
                this->memsize += 3*sizeof(Standard_Real);
                // if no geometry is attached to an edge an exception is raised
                const TopoDS_Edge& edge = TopoDS::Edge(shape);
                BRepAdaptor_Curve curve;
                try {
                    curve.Initialize(edge);
                }
                catch (const Standard_Failure&) {
                    continue;
                }
                switch (curve.GetType())
                {
                case GeomAbs_Line:
                    this->memsize += sizeof(Geom_Line);
                    break;
                case GeomAbs_Circle:
                    this->memsize += sizeof(Geom_Circle);
                    break;
                case GeomAbs_Ellipse:
                    this->memsize += sizeof(Geom_Ellipse);
                    break;
                case GeomAbs_Hyperbola:
                    this->memsize += sizeof(Geom_Hyperbola);
                    break;
                case GeomAbs_Parabola:
                    this->memsize += sizeof(Geom_Parabola);
                    break;
                case GeomAbs_BezierCurve:
                    this->memsize += sizeof(Geom_BezierCurve);
                    this->memsize += curve.NbPoles() * sizeof(Standard_Real);
                    this->memsize += curve.NbPoles() * sizeof(Geom_CartesianPoint);
                    break;
                case GeomAbs_BSplineCurve:
                    this->memsize += sizeof(Geom_BSplineCurve);
                    this->memsize += curve.NbKnots() * sizeof(Standard_Real);
                    this->memsize += curve.NbPoles() * sizeof(Standard_Real);
                    this->memsize += curve.NbPoles() * sizeof(Geom_CartesianPoint);
                    break;
                case GeomAbs_OtherCurve:
                    // What kind of curve should this be?
                    this->memsize += sizeof(Geom_Curve);
                    break;
                default:
                    break;
                }
            } break;
        case TopAbs_VERTEX:
            {
                // tolerance
                this->memsize += sizeof(Standard_Real);
                this->memsize += sizeof(Geom_CartesianPoint);
            } break;
        default:
            break;
        }
    }

    return this->memsize;
}

unsigned int TopoShape::getMemSize (void) const
{
    INIT_SHAPE_CACHE();
    return _Cache->getMemSize() + Data::ComplexGeoData::getMemSize();
}

TopoShape TopoShape::splitWires(std::vector<TopoShape> *inner,
                                SplitWireReorient reorient) const
{
    // ShapeAnalysis::OuterWire() is un-reliable for some reason. OCC source
    // code shows it works by creating face using each wire, and then test using
    // BRepTopAdaptor_FClass2d::PerformInfinitePoint() to check if it is an out
    // bound wire. And practice shows it sometimes returns the in correct
    // result.  Need more investigation. Note that this may be related to
    // unreliable solid face orientation
    // (https://forum.freecadweb.org/viewtopic.php?p=446006#p445674)
    //
    // Use BrepTools::OuterWire() instead. OCC source code shows it is
    // implemented using simple bound box checking. This should be a
    // reliable method, especially so for a planar face.

    TopoDS_Shape tmp;
    if (shapeType(true) == TopAbs_FACE)
        tmp = BRepTools::OuterWire(TopoDS::Face(_Shape));
    else if (countSubShapes(TopAbs_FACE) == 1)
        tmp = BRepTools::OuterWire(
                TopoDS::Face(getSubShape(TopAbs_FACE, 1)));
    if (tmp.IsNull())
        return TopoShape();
    const auto & wires = getSubTopoShapes(TopAbs_WIRE);
    auto it = wires.begin();

    TopAbs_Orientation orientOuter, orientInner;
    switch(reorient) {
    case ReorientReversed:
        orientOuter = orientInner = TopAbs_REVERSED;
        break;
    case ReorientForward:
        orientOuter = orientInner = TopAbs_FORWARD;
        break;
    default:
        orientOuter = TopAbs_FORWARD;
        orientInner = TopAbs_REVERSED;
        break;
    }

    auto doReorient = [](TopoShape &s, TopAbs_Orientation orient) {
        // Special case of single edge wire. Make sure the edge is in the
        // required orientation. This is necessary because BRepFill_OffsetWire
        // has special handling of circular edge offset, which seem to only
        // respect the edge orientation and disregard the wire orientation. The
        // orientation is used to determine whether to shrink or expand.
        if (s.countSubShapes(TopAbs_EDGE) == 1) {
            TopoDS_Shape e = s.getSubShape(TopAbs_EDGE, 1);
            if (e.Orientation() == orient) {
                if (s._Shape.Orientation() == orient)
                    return;
            } else
                e = e.Oriented(orient);
            BRepBuilderAPI_MakeWire mkWire(TopoDS::Edge(e));
            s.setShape(mkWire.Shape(), false);
        }
        else if (s._Shape.Orientation() != orient)
            s.setShape(s._Shape.Oriented(orient), false);
    };

    for (; it != wires.end(); ++it) {
        auto & wire = *it;
        if (wire.getShape().IsSame(tmp)) {
            if (inner) {
                for (++it; it != wires.end(); ++it) {
                    inner->push_back(*it);
                    if (reorient)
                        doReorient(inner->back(), orientInner);
                }
            }
            auto res = wire;
            if (reorient)
                doReorient(res, orientOuter);
            return res;
        }
        if (inner) {
            inner->push_back(wire);
            if (reorient)
                doReorient(inner->back(), orientInner);
        }
    }
    return TopoShape();
}

bool TopoShape::isLinearEdge() const
{
    if (isNull() || getShape().ShapeType() != TopAbs_EDGE)
        return false;

    return GeomCurve::isLinear(BRepAdaptor_Curve(TopoDS::Edge(getShape())).Curve().Curve());
}

bool TopoShape::isPlanarFace(double tol) const
{
    if (isNull() || getShape().ShapeType() != TopAbs_FACE)
        return false;

    return GeomSurface::isPlanar(
            BRepAdaptor_Surface(TopoDS::Face(getShape())).Surface().Surface(), nullptr, tol);
}

bool TopoShape::linearize(bool face, bool edge)
{
    bool touched = false;
    BRep_Builder builder;
    // Note: changing edge geometry seems to mess up with face (or shell, or solid)
    // Probably need to do some fix afterwards.
    if (edge) {
        for (auto & edge : getSubTopoShapes(TopAbs_EDGE)) {
            TopoDS_Edge e = TopoDS::Edge(edge.getShape());
            BRepAdaptor_Curve curve(e);
            if (curve.GetType() == GeomAbs_Line || !edge.isLinearEdge())
                continue;
            std::unique_ptr<Geometry> geo(
                    Geometry::fromShape(e.Located(TopLoc_Location()).Oriented(TopAbs_FORWARD)));
            std::unique_ptr<Geometry> gline(static_cast<GeomCurve*>(geo.get())->toLine());
            if (gline) {
                touched = true;
                builder.UpdateEdge(e,
                                   Handle(Geom_Curve)::DownCast(gline->handle()),
                                   e.Location(),
                                   BRep_Tool::Tolerance(e));
            }
        }
    }
    if (face) {
        for (auto & face : getSubTopoShapes(TopAbs_FACE)) {
            TopoDS_Face f = TopoDS::Face(face.getShape());
            BRepAdaptor_Surface surf(f);
            if (surf.GetType() == GeomAbs_Plane || !face.isPlanarFace())
                continue;
            std::unique_ptr<Geometry> geo(
                    Geometry::fromShape(f.Located(TopLoc_Location()).Oriented(TopAbs_FORWARD)));
            std::unique_ptr<Geometry> gplane(static_cast<GeomSurface*>(geo.get())->toPlane());
            if (gplane) {
                touched = true;
                builder.UpdateFace(f, 
                                Handle(Geom_Surface)::DownCast(gplane->handle()),
                                f.Location(),
                                BRep_Tool::Tolerance(f));
            }
        }
    }
    return touched;
}
