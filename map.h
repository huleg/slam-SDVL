/*
 *  Copyright (C) 1997-2017 JdeRobot Developers Team
 *
 *  This program is free software; you can redistribute it and/or modifdisty
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *   Authors : Eduardo Perdices <eperdices@gsyc.es>
 *
 */

#ifndef SDVL_MAP_H_
#define SDVL_MAP_H_

#include <iostream>
#include <vector>
#include <queue>
#include <pthread.h>
#include <memory>
#include "./frame.h"
#include "./matcher.h"
#include "./point.h"
#include "extra/orb_detector.h"

namespace sdvl {

class Frame;
class Point;
class Feature;

class Map {
 public:
  explicit Map(int cell_size);
  ~Map();

  // Start/Stop Map thread
  void Start();
  void Stop();
  int Run();

  std::vector<std::shared_ptr<Frame>>& GetKeyframes() { return keyframes_; }

  // Map locks
  inline void Lock() { pthread_mutex_lock(&mutex_map_); }
  inline void Unlock() { pthread_mutex_unlock(&mutex_map_); }

  // Update map
  void UpdateMap();

  // Add a new keyframe to keyframes list
  void AddKeyframe(const std::shared_ptr<Frame> &frame, bool search = true);

  // Add a new frame to candidate search queue
  void AddFrame(const std::shared_ptr<Frame> &frame);

  // Add a new candidate
  inline void AddCandidate(const std::shared_ptr<Point> &candidate) {
    candidates_.push_back(candidate);
  }

  // Set relocalizing
  inline void SetRelocalizing(bool v) { relocalizing_ = v; }

  // Detete a point
  void DeletePoint(const std::shared_ptr<Point> &point);

  // Return true if we need a new keyframe taking into account matches distribution
  bool NeedKeyframe(const std::shared_ptr<Frame> &frame, int matches);

  // Remove furthest keyframe if we have too many
  void LimitKeyframes(const std::shared_ptr<Frame> &frame);

  // Emptry points trash
  void EmptyTrash();

  // Transform initial map to set a z=0 plane
  bool TransformInitialMap(const std::shared_ptr<Frame> &frame);

  // Reset selected points
  void ResetSelected();

  // Perform bundle adjustment
  void BundleAdjustment();

 private:
  // Create new candidates from given frame
  void InitCandidates(const std::shared_ptr<Frame> &frame);

  // Update current candidates with a new frame
  void UpdateCandidates(const std::shared_ptr<Frame> &frame);

  // Check connections between a new keyframe and old keyframes
  void CheckConnections(const std::shared_ptr<Frame> &frame);

  // Return keyframe furthest apart from input frame
  std::shared_ptr<Frame> GetFurthestKeyframe(const std::shared_ptr<Frame> &frame);

  ORBDetector orb_detector_;  // ORB detector

  std::vector<std::shared_ptr<Frame>> keyframes_;     // List of keyframes
  std::queue<std::shared_ptr<Frame>> frame_queue_;    // Queue of frames to search candidates
  std::vector<std::shared_ptr<Frame>> frame_trash_;   // Frames trash

  std::vector<std::shared_ptr<Point>> candidates_;       // Points not converged
  std::vector<std::shared_ptr<Point>> points_trash_;     // Points trash

  bool new_keyframe_set_;           // True if we have got a new keyframe
  bool candidates_updating_halt_;   // Stops candidates updating
  int n_initializations_;           // Number of keyframes where candidates where created
  int cell_size_;                   // Grid cell size
  bool relocalizing_;               // True if tracking is relocalizing
  std::shared_ptr<Frame> ba_kf_;    // Keyframe to do bundle adjuntment
  std::shared_ptr<Frame> last_kf_;  // Last Keyframe saved
  int last_matches_;                // Matches found in last iteration

  pthread_t thread_;
  bool running_;
  pthread_mutex_t mutex_map_;     // Main map mutex
};

}  // namespace sdvl

#endif  // SDVL_MAP_H_