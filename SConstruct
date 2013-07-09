# Copyright 2013, Qualcomm Innovation Center, Inc.
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
#

import os

# include the core
env = SConscript(['../build_core/SConscript'])

# If building the the common project without building alljoyn_core status.h/.cc
# will not be built typically because the status.h/.cc file will be built from
# the alljoyn_core project and will include more status parameters.
# the BUILD_COMMON_STATUS should only be defined if building common project only
vars = Variables()
vars.Add('BUILD_COMMON_STATUS', 'When building the common build status.h and status.cc using the status.xml from common', '1')
vars.Update(env)

# Add/remove projects from build
env.SConscript(['SConscript'])


