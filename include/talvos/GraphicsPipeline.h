// Copyright (c) 2018 the Talvos developers. All rights reserved.
//
// This file is distributed under a three-clause BSD license. For full license
// terms please see the LICENSE file distributed with this source code.

/// \file GraphicsPipeline.h
/// This file declares the GraphicsPipeline class.

#ifndef TALVOS_GRAPHICSPIPELINE_H
#define TALVOS_GRAPHICSPIPELINE_H

namespace talvos
{

class PipelineStage;

/// This class encapsulates a graphics pipeline.
class GraphicsPipeline
{
public:
  /// Create a graphics pipeline.
  /// Ownership of any non-null stages is transferred to the pipeline.
  GraphicsPipeline(PipelineStage *VertexStage, PipelineStage *FragmentStage)
      : VertexStage(VertexStage), FragmentStage(FragmentStage){};

  /// Destroy the pipeline.
  ~GraphicsPipeline();

  // Do not allow GraphicsPipeline objects to be copied.
  ///\{
  GraphicsPipeline(const GraphicsPipeline &) = delete;
  GraphicsPipeline &operator=(const GraphicsPipeline &) = delete;
  ///\}

  /// Returns the fragment pipeline stage.
  const PipelineStage *getFragmentStage() const { return FragmentStage; }

  /// Returns the vertex pipeline stage.
  const PipelineStage *getVertexStage() const { return VertexStage; }

private:
  /// The vertex pipeline stage in this pipeline.
  PipelineStage *VertexStage;

  /// The fragment pipeline stage in this pipeline.
  PipelineStage *FragmentStage;
};

} // namespace talvos

#endif
