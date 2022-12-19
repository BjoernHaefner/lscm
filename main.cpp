/*
 *  Copyright (c) 2012-2014, Bruno Levy
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *  this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *  this list of conditions and the following disclaimer in the documentation
 *  and/or other materials provided with the distribution.
 *  * Neither the name of the ALICE Project-Team nor the names of its
 *  contributors may be used to endorse or promote products derived from this
 *  software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 *  If you modify this software, you should include a notice giving the
 *  name of the person performing the modification, the date of modification,
 *  and the reason for such modification.
 *
 *  Contact: Bruno Levy
 *
 *     Bruno.Levy@inria.fr
 *     http://www.loria.fr/~levy
 *
 *     ALICE Project
 *     LORIA, INRIA Lorraine,
 *     Campus Scientifique, BP 239
 *     54506 VANDOEUVRE LES NANCY CEDEX
 *     FRANCE
 *
 */

// A mesh file converter using Geogram.

#include <geogram/basic/command_line.h>
#include <geogram/basic/command_line_args.h>
#include <geogram/basic/logger.h>
#include <geogram/basic/stopwatch.h>
#include <geogram/image/image.h>
#include <geogram/image/image_library.h>
#include <geogram/mesh/mesh_baking.h>
#include <geogram/mesh/mesh_decimate.h>
#include <geogram/mesh/mesh_geometry.h>
#include <geogram/mesh/mesh_intersection.h>
#include <geogram/mesh/mesh_io.h>
#include <geogram/mesh/mesh_preprocessing.h>
#include <geogram/mesh/mesh_repair.h>
#include <geogram/parameterization/mesh_LSCM.h>
#include <geogram/parameterization/mesh_atlas_maker.h>

#include <memory>

void LoadMesh(GEO::Mesh& M, const std::string& filename) {
  GEO::Logger::div("Loading");
  {
    GEO::Stopwatch W("load");
    GEO::MeshIOFlags ioflags;
    ioflags.set_attribute(GEO::MESH_ALL_ATTRIBUTES);
    if (!GEO::mesh_load(filename, M, ioflags)) {
      return;
    }
  }
}

void RepairMesh(GEO::Mesh& M, bool remove_intersection = false) {
  GEO::Logger::div("Repair mesh");
  {
    //    GEO::mesh_repair(M);
    //    if (remove_intersection) {
    //      GEO::mesh_remove_intersections(M);
    //    }
    GEO::CmdLine::import_arg_group("algo");
    GEO::mesh_repair(
        M,
        GEO::MeshRepairMode(GEO::MESH_REPAIR_COLOCATE | GEO::MESH_REPAIR_DUP_F),
        1e-3 * surface_average_edge_length(M));
    tessellate_facets(M, 3);
    mesh_remove_intersections(M);

    M.show_stats();
  }
}

void PreprocessMesh(GEO::Mesh& M, double rel_area = 0.0) {
  GEO::Logger::div("Preprocess mesh");
  {
    if (rel_area > 0.0) {
      GEO::remove_small_connected_components(
          M, rel_area * GEO::Geom::mesh_area(M));
      //      GEO::CmdLine::import_arg_group("algo");
      //      GEO::fill_holes(M, rel_area);
    }

    M.show_stats();
  }
}

bool DecimateMesh(GEO::Mesh& M, GEO::index_t nb_bins) {
  GEO::Logger::div("Decimate mesh");
  {
    GEO::MeshDecimateMode mode;
    //  mode = GEO::MESH_DECIMATE_FAST;
    mode = GEO::MESH_DECIMATE_DUP_F;
    //    mode = GEO::MESH_DECIMATE_DEG_3;
    //    mode = GEO::MESH_DECIMATE_KEEP_B;
    //    mode = GEO::MESH_DECIMATE_DEFAULT;
    GEO::mesh_decimate_vertex_clustering(M, nb_bins, mode);
    mode = GEO::MESH_DECIMATE_DEG_3;
    GEO::mesh_decimate_vertex_clustering(M, nb_bins, mode);
  }
  return true;
}

void CalculateTextureAtlas(GEO::Mesh& M) {
  GEO::Logger::div("Calculate Texture Atlas");
  {
    double hard_angles_threshold = 45.0;
    GEO::ChartParameterizer param =
        GEO::PARAM_LSCM;  // PARAM_LSCM, PARAM_SPECTRAL_LSCM, PARAM_ABF
    GEO::ChartPacker pack = GEO::PACK_XATLAS;  // PACK_TETRIS, PACK_XATLAS
    bool verbose = false;
    GEO::mesh_make_atlas(M, hard_angles_threshold, param, pack, verbose);
    M.show_stats();
  }
}

void BakeMeshFacetNormals(GEO::Mesh& M, const std::string& filename,
                          GEO::index_t size) {
  GEO::Logger::div("Bake facet normals");
  {
    std::unique_ptr<GEO::Image> target(
        new GEO::Image(GEO::Image::ColorEncoding::RGB,
                       GEO::Image::ComponentEncoding::BYTE, size, size));
    GEO::bake_mesh_facet_normals(&M, target.get());

    GEO::ImageLibrary* il = GEO::ImageLibrary::instance();
    il->save_image(filename, target.get());
  }
}

void BakeAndSaveTextureAtlas(GEO::Mesh& M, const std::string& filename,
                             GEO::index_t size) {
  GEO::Logger::div("Bake Texture Atlas");
  {
    std::unique_ptr<GEO::Image> target(
        new GEO::Image(GEO::Image::ColorEncoding::RGB,
                       GEO::Image::ComponentEncoding::BYTE, size, size));
    GEO::Attribute<double> attribute(M.vertices.attributes(), "color");
    GEO::bake_mesh_attribute(&M, target.get(), attribute);

    GEO::ImageLibrary* il = GEO::ImageLibrary::instance();
    il->save_image(filename, target.get());
  }
}

void SaveMesh(const GEO::Mesh& M, const std::string& mesh_name,
              const std::string& tex_name) {
  GEO::Logger::div("Saving");
  {
    GEO::Stopwatch W("save");
    GEO::MeshIOFlags ioflags;
    ioflags.set_texture_filename(tex_name);
    if (!GEO::mesh_save(M, mesh_name, ioflags)) {
      return;
    }
    M.show_stats();
  }
}

int main(int argc, char** argv) {
  // Initialize the Geogram library.
  GEO::initialize();

  // Import standard command line arguments.
  GEO::CmdLine::import_arg_group("standard");

  // Parse command line options and filenames.
  std::vector<std::string> filenames;
  if (!GEO::CmdLine::parse(
          argc, argv, filenames,
          "in_mesh_file out_mesh_file out_tex_map out_normal_map")) {
    return 1;
  }

  // Default output filename is "out.meshb" if unspecified.
  if (filenames.size() == 1) {
    filenames.push_back("out.meshb");
  }

  // Display input and output filenames.
  GEO::Logger::div("Command line");
  GEO::Logger::out("MyApp") << "Input mesh: " << filenames[0] << std::endl;
  GEO::Logger::out("MyApp") << "Output mesh: " << filenames[1] << std::endl;
  GEO::Logger::out("MyApp")
      << "Output texture map: " << filenames[2] << std::endl;
  GEO::Logger::out("MyApp")
      << "Output normal map: " << filenames[3] << std::endl;

  // Declare a mesh.
  GEO::Mesh M;

  LoadMesh(M, filenames[0]);
  RepairMesh(M);
  PreprocessMesh(M, 0.01);

  GEO::orient_normals(M);
  CalculateTextureAtlas(M);

  if (filenames.size() >= 4) {
    BakeMeshFacetNormals(M, filenames[3], 1024);
  }

  if (filenames.size() >= 3) {
    BakeAndSaveTextureAtlas(M, filenames[2], 1024);
    SaveMesh(M, filenames[1], filenames[2]);
  } else {
    SaveMesh(M, filenames[1], "");
  }

  return 0;
}
