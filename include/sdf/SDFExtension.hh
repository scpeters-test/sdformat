/*
 * Copyright 2015 Open Source Robotics Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
*/

#ifndef _SDFORMAT_SDFEXTENSION_HH_
#define _SDFORMAT_SDFEXTENSION_HH_

#include <tinyxml.h>
#include <string>
#include <vector>

#include "sdf/Types.hh"

namespace sdf
{
  /// \internal
  /// \brief A class for holding sdf extension elements in urdf
  class SDFExtension
  {
    /// \brief Constructor
    public: SDFExtension();

    /// \brief Copy constructor
    /// \param[in] _ge SDFExtension to copy.
    public: SDFExtension(const SDFExtension &_ge);

    /// \brief Destructor
    public: virtual ~SDFExtension() = default;

    // for reducing fixed joints and removing links
    public: std::string oldLinkName;
    public: sdf::Pose reductionTransform;

    // visual
    public: std::string material;
    public: std::vector<boost::shared_ptr<TiXmlElement> > visual_blobs;

    // body, default off
    public: bool setStaticFlag;
    public: bool gravity;
    public: bool isDampingFactor;
    public: double dampingFactor;
    public: bool isMaxContacts;
    public: int maxContacts;
    public: bool isMaxVel;
    public: double maxVel;
    public: bool isMinDepth;
    public: double minDepth;
    public: bool selfCollide;

    // geom, contact dynamics
    public: bool isMu1, isMu2, isKp, isKd;
    public: double mu1, mu2, kp, kd;
    public: std::string fdir1;
    public: bool isLaserRetro;
    public: double laserRetro;

    // joint, joint limit dynamics
    public: bool isStopCfm, isStopErp, isInitialJointPosition, isFudgeFactor;
    public: double stopCfm, stopErp, initialJointPosition, fudgeFactor;
    public: bool isProvideFeedback;
    public: bool provideFeedback;
    public: bool isImplicitSpringDamper;
    public: bool implicitSpringDamper;
    public: bool isStopKp, isStopKd;
    public: double stopKp, stopKd;

    // blobs into body or robot
    public: std::vector<boost::shared_ptr<TiXmlElement> > blobs;

    friend class SDFORMAT_VISIBLE URDF2SDF;
  };
}
#endif
