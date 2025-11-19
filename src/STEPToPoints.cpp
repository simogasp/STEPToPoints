//
// Program: STEPToPoints
//
// Description:
//
// The program STEPToPoints converts solids contained in STEP files into point clouds by regular sampling.
//
// Copyright(C) 2022 Alexander Leutgeb
//
// This library is free software; you can redistribute it and / or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110 - 1301  USA
//

#include <TopoDS_Solid.hxx>
#include <TopoDS_Compound.hxx>
#include <TopoDS_Builder.hxx>
#include <TopoDS.hxx>
#include <STEPCAFControl_Reader.hxx>
#include <XCAFApp_Application.hxx>
#include <XCAFDoc_DocumentTool.hxx>
#include <XCAFDoc_ShapeTool.hxx>
#include <TDocStd_Document.hxx>
#include <TDF_ChildIterator.hxx>
#include <TDF_LabelSequence.hxx>
#include <TDataStd_Name.hxx>
#include <IntCurvesFace_ShapeIntersector.hxx>
#include <Bnd_Box.hxx>
#include <BRepBndLib.hxx>
#include <gp_Lin.hxx>
#include <BRepTools.hxx>
#include <BRep_Tool.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <GeomLProp_SLProps.hxx>
#include <indicators/block_progress_bar.hpp>
#include <indicators/cursor_control.hpp>
#include "cxxopts.hpp"
#include "Timer.hpp"
#include "happly.hpp"
#include "solid_index_parser.hpp"
#include <algorithm>
#include <vector>
#include <set>
#include <array>
#include <numeric>
#include <format>
#include <execution>
#include <thread>
#include <iostream>
#include <filesystem>
#include <optional>
#include <ranges>


struct NamedSolid
{
    NamedSolid(const TopoDS_Solid& s, const std::string& n)
        : solid{s}, name{n} {}

    const TopoDS_Solid solid;
    const std::string name;
};

void getNamedSolids(const TopLoc_Location& location,
                    const std::string& prefix,
                    unsigned int& id,
                    const Handle(XCAFDoc_ShapeTool) shapeTool,
                    const TDF_Label label,
                    std::vector<NamedSolid>& namedSolids)
{
    TDF_Label referredLabel{label};
    if(XCAFDoc_ShapeTool::IsReference(label))
    {
        XCAFDoc_ShapeTool::GetReferredShape(label, referredLabel);
    }
    std::string name;

    if(Handle(TDataStd_Name) shapeName; referredLabel.FindAttribute(TDataStd_Name::GetID(), shapeName))
    {
        name = TCollection_AsciiString(shapeName->Get()).ToCString();
    }
    if(name.empty())
    {
        name = std::to_string(id);
        id++;
    }
    std::string fullName{prefix + "/" + name};

    const TopLoc_Location localLocation = location * XCAFDoc_ShapeTool::GetLocation(label);
    TDF_LabelSequence components;
    if(XCAFDoc_ShapeTool::GetComponents(referredLabel, components))
    {
        for(Standard_Integer compIndex{1}; compIndex <= components.Length(); ++compIndex)
        {
            getNamedSolids(localLocation, fullName, id, shapeTool, components.Value(compIndex), namedSolids);
        }
    }
    else
    {
        TopoDS_Shape shape;
        XCAFDoc_ShapeTool::GetShape(referredLabel, shape);
        if(shape.ShapeType() == TopAbs_SOLID)
        {
            BRepBuilderAPI_Transform transform(shape, localLocation, Standard_True);
            namedSolids.emplace_back(TopoDS::Solid(transform.Shape()), fullName);
        }
    }
}

[[nodiscard]] auto readSolids(const std::string& inFile) -> std::vector<NamedSolid>
{
    std::vector<NamedSolid> namedSolids{};
    Handle(TDocStd_Document) document;
    Handle(XCAFApp_Application) application = XCAFApp_Application::GetApplication();
    std::cout << "Reading " << inFile << "\n";
    application->NewDocument(inFile.c_str(), document);
    STEPCAFControl_Reader reader;
    reader.SetNameMode(true);
    if(const auto stat{reader.ReadFile(inFile.c_str())}; stat != IFSelect_RetDone || !reader.Transfer(document))
    {
        throw std::invalid_argument{"Could not read " + inFile};
    }
    Handle(XCAFDoc_ShapeTool) shapeTool{XCAFDoc_DocumentTool::ShapeTool(document->Main())};
    TDF_LabelSequence topLevelShapes;
    shapeTool->GetFreeShapes(topLevelShapes);
    const auto numLevels = topLevelShapes.Length();
    namedSolids.reserve(static_cast<std::size_t>(topLevelShapes.Length()));
    unsigned int id{1};
    for(Standard_Integer iLabel{1}; iLabel <= numLevels; ++iLabel)
    {
        getNamedSolids(TopLoc_Location{}, "", id, shapeTool, topLevelShapes.Value(iLabel), namedSolids);
    }
    return namedSolids;
}

struct Point
{
    Point(const std::array<double, 3>& v, const std::array<double, 3>& n)
        : vertex{v}, normal{n} {}

    std::array<double, 3> vertex;
    std::array<double, 3> normal;
};

/**
 * @brief Writes a point cloud to a `.xyz` file.
 *
 * This function takes a vector of 3D points with normals and writes them
 * to a file in the `.xyz` format. Each line in the file contains the
 * coordinates of a point followed by its normal vector components.
 *
 * @param[in] outFile The path to the output `.xyz` file.
 * @param[in] points A vector of `Point` objects representing the point cloud.
 */
void writeXYZ(const std::string& outFile, const std::vector<Point>& points)
{
    std::ofstream ofs{outFile};
    for(const auto& p : points)
    {
        ofs << p.vertex[0] << " " << p.vertex[1] << " " << p.vertex[2] << " " << p.normal[0] << " " << p.normal[1] <<
            " " << p.normal[2] << "\n";
    }
    ofs.close();
}

void writeOBJ(const std::string& outFile, const std::vector<Point>& points)
{
    std::ofstream ofs{outFile};
    if(!ofs.is_open())
    {
        throw std::invalid_argument{std::format("Could not open {}", outFile)};
    }
    // write the points in wavefront obj format
    for(const auto& p : points)
    {
        ofs << "v " << p.vertex[0] << " " << p.vertex[1] << " " << p.vertex[2] << "\nvn " << p.normal[0] << " " << p.normal[1] <<
            " " << p.normal[2] << "\n";
    }
    ofs.close();
}

void addProperty(happly::PLYData& plyOut,
                 const std::vector<Point>& points,
                 const std::string& vertexName,
                 const std::string& propertyName,
                 const std::function<double(const Point&)>& extractor)
{
    std::vector<double> values;
    values.reserve(points.size());
    std::ranges::transform(points, std::back_inserter(values), extractor);
    plyOut.getElement(vertexName).addProperty<double>(propertyName, values);
}

void writePLY(const std::string& outFile, const std::vector<Point>& points, bool binary)
{
    const std::string vertexName{"vertex"};
    const auto numPoints{points.size()};
    // Create a PlyData object
    happly::PLYData plyOut;

    plyOut.addElement(vertexName, numPoints);

    // Add vertex coordinates
    addProperty(plyOut, points, vertexName, "x", [](const Point& p) { return p.vertex[0]; });
    addProperty(plyOut, points, vertexName, "y", [](const Point& p) { return p.vertex[1]; });
    addProperty(plyOut, points, vertexName, "z", [](const Point& p) { return p.vertex[2]; });

    // Add normals
    addProperty(plyOut, points, vertexName, "nx", [](const Point& p) { return p.normal[0]; });
    addProperty(plyOut, points, vertexName, "ny", [](const Point& p) { return p.normal[1]; });
    addProperty(plyOut, points, vertexName, "nz", [](const Point& p) { return p.normal[2]; });

    // Write to file
    const auto format = binary ? happly::DataFormat::Binary : happly::DataFormat::ASCII;
    plyOut.write(outFile, format);
}


void savePointCloud(const std::string& outFile, const std::vector<Point>& points, bool binary)
{
    namespace fs = std::filesystem;
    const auto ext = fs::path(outFile).extension();
    if(ext.empty())
    {
        throw std::invalid_argument{"Output file has no extension: " + outFile};
    }
    if(ext == ".xyz")
    {
        const std::string opt_message{binary ? " (binary option ignored for .xyz files)" : ""};
        std::cout << "Writing XYZ file: " << outFile << opt_message<< "\n";
        writeXYZ(outFile, points);
    }
    else if(ext == ".obj")
    {
        const std::string opt_message{binary ? " (binary option ignored for .obj files)" : ""};
        std::cout << "Writing OBJ file: " << outFile << opt_message<< "\n";
        writeOBJ(outFile, points);
    }
    else if(ext == ".ply")
    {
        const std::string opt_message{binary ? " with binary option" : ""};
        std::cout << "Writing PLY file: " << outFile << opt_message<< "\n";
        writePLY(outFile, points, binary);
    }
    else
    {
        throw std::invalid_argument{"Unknown file extension: " + ext.string()};
    }
}

struct PointLessOperator
{
    explicit PointLessOperator(const double e)
        : eps{e} {}

    bool operator()(const Point& lhs, const Point& rhs) const
    {
        for(auto i{0u}; i < 3; ++i)
        {
            if(std::abs(lhs.vertex[i] - rhs.vertex[i]) > eps)
                return lhs.vertex[i] < rhs.vertex[i];
        }
        return false;
    }

    const double eps;
};

auto makeUniquePoints(const std::vector<Point>& points, const double epsilon)
{
    std::vector<Point> result;

    PointLessOperator lessOperator{epsilon};
    std::set<Point, decltype(lessOperator)> pointSet{lessOperator};
    for(const auto& p : points)
    {
        pointSet.insert(p);
    }
    result.reserve(pointSet.size());
    for(const auto& key : pointSet)
    {
        result.emplace_back(key);
    }
    return result;
}

auto createScanLines(const TopoDS_Shape& shape, const double sampling) -> std::vector<gp_Lin>
{
    std::vector<gp_Lin> result;
    std::array<double, 3> min{};
    std::array<double, 3> max{};
    Bnd_Box box;
    BRepBndLib::Add(shape, box);
    box.Get(min[0], min[1], min[2], max[0], max[1], max[2]);
    std::cout << "Bounding box size: x = " << max[0] - min[0] << ", y = " << max[1] - min[1] << ", z = " << max[2] - min[2]
              << "\n";
    for(auto dim{0u}; dim < 3u; ++dim)
    {
        const double uMin{min[(dim + 1) % 3]};
        const double uMax{max[(dim + 1) % 3]};
        const double vMin{min[(dim + 2) % 3]};
        const double vMax{max[(dim + 2) % 3]};
        std::array<double, 3> direction{0.0, 0.0, 0.0};
        direction[dim] = 1.0;
        std::array<double, 3> point{min};
        for(double u{uMin}; u < uMax; u += sampling)
        {
            for(double v{vMin}; v < vMax; v += sampling)
            {
                point[(dim + 1) % 3] = u;
                point[(dim + 2) % 3] = v;
                result.emplace_back(gp_Pnt{point[0], point[1], point[2]},
                                    gp_Dir{direction[0], direction[1], direction[2]});
            }
        }
    }
    return result;
}

auto surfaceNormal(const TopoDS_Face& face, const double u, const double v, const double resolution) -> gp_Dir
{
    Handle(Geom_Surface) surface{BRep_Tool::Surface(face)};
    GeomLProp_SLProps props{surface, u, v, 1, resolution};
    gp_Dir normal{props.Normal()};
    if(face.Orientation() == TopAbs_REVERSED)
    {
        normal.Reverse();
    }
    return normal;
}

auto sampleShape(const TopoDS_Shape& shape, const double sampling) -> std::vector<Point>
{
    std::vector<Point> result;
    std::vector<gp_Lin> scanLines = createScanLines(shape, sampling);
    const auto numScanLines{scanLines.size()};
    std::cout << "Created " << numScanLines << " scanlines\n";
    const double tolerance{sampling * 0.001};
    const auto numThreads{std::thread::hardware_concurrency()};

    std::cout << "Using " << numThreads << " threads\n";

    std::vector<IntCurvesFace_ShapeIntersector> tlsIntersectors(numThreads);
    for(auto& intersector : tlsIntersectors)
    {
        intersector.Load(shape, tolerance);
    }

    std::vector<std::vector<Point>> tlsResult(numThreads);
    std::vector<std::vector<gp_Lin>> tlsScanLines(numThreads);
    for(std::size_t i{0}; i < scanLines.size(); ++i)
    {
        tlsScanLines[i % numThreads].emplace_back(scanLines[i]);
    }
    std::vector<unsigned int> threadIDs(numThreads);
    std::iota(std::begin(threadIDs), std::end(threadIDs), 0);

    // @TODO reserve the proper amount of memory for the results
    for(auto& res : tlsResult)
    {
        res.reserve(10000);
    }

    namespace ind = indicators;
    ind::show_console_cursor(false);

    // @TODO determine a proper granularity
    const auto granularity{100u};
    const auto max_progress = numScanLines / granularity;
    std::cout << "Sampling points on the surface...\n";
    ind::BlockProgressBar bar{
        ind::option::BarWidth{100u},
        ind::option::Start{"["},
        ind::option::End{"]"},
        ind::option::ForegroundColor{ind::Color::white}  ,
        ind::option::ShowPercentage{true},
        ind::option::ShowElapsedTime{true},
        ind::option::ShowRemainingTime{true},
        ind::option::MaxProgress{max_progress},
        ind::option::FontStyles{std::vector{ind::FontStyle::bold}}
    };
    std::atomic processedScanLines = decltype(numScanLines){0};
    Timer timer{};
    timer.start();

    // ugly cast because Windoze requires a signed integer for the loop variable
    #pragma omp parallel for num_threads(numThreads)
    for(int t_idx = 0; t_idx < static_cast<int>(threadIDs.size()); ++t_idx)
    {
        const auto threadID = threadIDs[static_cast<std::size_t>(t_idx)];
        auto processed{0u};
        for(const auto& scanLine : tlsScanLines[threadID])
        {
            auto& intersector{tlsIntersectors[threadID]};
            intersector.Perform(scanLine, -RealLast(), RealLast());
            if(!intersector.IsDone())
                continue;
            for(auto i{1}; i <= intersector.NbPnt(); ++i)
            {
                const gp_Pnt p{intersector.Pnt(i)};
                const TopoDS_Face f{intersector.Face(i)};
                const gp_Dir n{
                    surfaceNormal(f, intersector.UParameter(i), intersector.VParameter(i), tolerance)};
                tlsResult[threadID].emplace_back(std::array<double, 3>{p.X(), p.Y(), p.Z()},
                                                 std::array<double, 3>{n.X(), n.Y(), n.Z()});
            }
            ++processed;
            if(processed % granularity == 0)
            {
                processedScanLines+=granularity;
                bar.tick();
                bar.set_option(ind::option::PostfixText{std::format("{} / {}", processedScanLines.load(), numScanLines)});
            }
        }
    }

    timer.stop();
    bar.mark_as_completed();
    ind::show_console_cursor(true);
    std::cout << "Sampling took " << timer.elapsed_seconds() << " seconds\n";
    for(const auto& r : tlsResult)
    {
        std::ranges::copy(r, std::back_inserter(result));
    }
    return result;
}

/**
 * @brief Finds a solid by its full name path.
 *
 * @param namedSolids The vector of named solids to search.
 * @param name The full path name to search for (must start with '/').
 * @return std::optional<std::reference_wrapper<const NamedSolid>> Reference to the found solid or std::nullopt.
 */
auto findSolidByName(const std::vector<NamedSolid>& namedSolids, const std::string& name)
    -> std::optional<std::reference_wrapper<const NamedSolid>>
{
    const auto it = std::ranges::find_if(namedSolids, [&name](const auto& ns) {
        return ns.name == name;
    });

    if(it != namedSolids.end())
    {
        return std::cref(*it);
    }
    return std::nullopt;
}

/**
 * @brief Resolves a selection string to a solid.
 *
 * @param sel The selection string (either a name starting with '/' or a 1-based index or range).
 * @param namedSolids The vector of all available named solids.
 * @return std::vector<std::reference_wrapper<const NamedSolid>> List of references to the selected solid(s).
 * @throws std::invalid_argument If the selection cannot be resolved.
 */
auto resolveSolidSelection(const std::string& sel, const std::vector<NamedSolid>& namedSolids)
    -> std::vector<std::reference_wrapper<const NamedSolid>>
{
    if(sel.empty())
    {
        throw std::invalid_argument{"Empty selection string"};
    }

    // Check if it's a name (starts with '/')
    if(sel[0] == '/')
    {
        if(const auto found = findSolidByName(namedSolids, sel))
        {
            return {found->get()};
        }
        throw std::invalid_argument{std::format("Could not find solid with name '{}'", sel)};
    }

    // Try to parse as index
    if(const auto indices = parseSolidIndex(sel, namedSolids.size()); indices.has_value())
    {
        std::vector<std::reference_wrapper<const NamedSolid>> result;
        result.reserve(indices->size());
        for(const auto idx : indices.value())
        {
            result.emplace_back(std::cref(namedSolids[idx]));
        }
        return result;
    }

    throw std::invalid_argument{std::format("Invalid selection: '{}' (not a valid name or index)", sel)};
}

/**
 * @brief Builds a compound shape from selected solids.
 *
 * @param namedSolids The vector of all available named solids.
 * @param selections The list of selection strings (names or indices). If empty, all solids are included.
 * @return TopoDS_Compound A compound shape containing the selected solids.
 * @throws std::invalid_argument If any selection is invalid.
 */
auto buildCompoundFromSelections(const std::vector<NamedSolid>& namedSolids,
                                  const std::vector<std::string>& selections) -> TopoDS_Compound
{
    TopoDS_Compound compound;
    TopoDS_Builder builder;
    builder.MakeCompound(compound);

    if(selections.empty())
    {
        std::cout << "No selections provided, adding all solids.\n";
        // Add all solids
        for(const auto& namedSolid : namedSolids)
        {
            builder.Add(compound, namedSolid.solid);
        }
    }
    else
    {
        // Add only selected solids
        for(const auto& sel : selections)
        {
            if(!sel.empty())
            {
                const auto& selectedSolid = resolveSolidSelection(sel, namedSolids);
                for(const auto& solid : selectedSolid)
                {
                    std::cout << "Adding solid: " << solid.get().name << "\n";
                    builder.Add(compound, solid.get().solid);
                }
            }
        }
    }

    return compound;
}

void write(const std::string& outFile,
           const std::vector<NamedSolid>& namedSolids,
           const std::vector<std::string>& select,
           double sampling,
           bool binary)
{
    const auto compound = buildCompoundFromSelections(namedSolids, select);
    const auto points = makeUniquePoints(sampleShape(compound, sampling), sampling * 0.001);

    std::cout << "\nCreated " << points.size() << " points\n";
    Timer timer{};
    timer.start();
    savePointCloud(outFile, points, binary);
    timer.stop();
    std::cout << "Saved point cloud in " << outFile << " in " << timer.elapsed_seconds() << " seconds\n";
}

int main(int argc, char* argv[])
{
    cxxopts::Options options{"STEPToPoints", "STEP to point cloud conversion by regular sampling"};
    options.
        add_options()
    ("i,in", "Input file", cxxopts::value<std::string>())
    ("o,out",
    "Output file (Supported formats: .xyz, .ply, .obj)",
    cxxopts::value<std::string>())
    ("c,content", "List content (solids)")
    ("s,select",
    "Select solids by name or index (comma separated list, index starts with 1). For the indices, the range is also supported, e.g. `1-3` for the first three solids.",
    cxxopts::value<std::vector<std::string>>())
    ("g,sampling", "Sampling distance", cxxopts::value<double>())
    ("b,binary", "Write binary file (only for .ply files)", cxxopts::value<bool>()->default_value("false"))
    ("h,help", "Print usage");

    if(const auto result{options.parse(argc, argv)}; result.count("help"))
    {
        std::cout << options.help() << std::endl;
    }
    else if(result.count("content") && result.count("in"))
    {
        const std::string inFile{result["in"].as<std::string>()};
        const std::vector<NamedSolid> namedSolids = readSolids(inFile);
        for(auto i{0u}; i < namedSolids.size(); ++i)
        {
            std::cout << (i + 1) << "\t" << namedSolids[i].name << std::endl;
        }

    }
    else if(result.count("in") && result.count("out") && result.count("sampling"))
    {
        const auto inFile = result["in"].as<std::string>();
        const auto outFile = result["out"].as<std::string>();
        const auto sampling = result["sampling"].as<double>();
        std::vector<std::string> select;
        if(result.count("select"))
        {
            select = result["select"].as<std::vector<std::string>>();
        }
        const std::vector<NamedSolid> namedSolids = readSolids(inFile);
        const auto write_binary = result["binary"].as<bool>();
        write(outFile, namedSolids, select, sampling, write_binary);
    }
    else
    {
        std::cout << options.help() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}