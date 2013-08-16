# Copyright 2010 - 2013 Qualcomm Innovation Center, Inc.
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
Import('env')
from os.path import basename

#default crypto for most platforms is openssl
env['CRYPTO'] = 'openssl'

# Bullseye code coverage for 'debug' builds.
if env['VARIANT'] == 'debug':
    if not env.has_key('BULLSEYE_BIN'):
        print 'BULLSEYE_BIN not specified'
    else:
        env.PrependENVPath('PATH', env.get('BULLSEYE_BIN'))
        if not os.environ.has_key('COVFILE'):
            print 'Error: COVFILE environment variable must be set'
            if not GetOption('help'):
                Exit()
        else:
            env.PrependENVPath('COVFILE', os.environ['COVFILE'])


# All AllJoyn subprojects have access to common so add the include path to the global environment
env.Append(CPPPATH = [env.Dir('inc')])

# Platform specifics for common
if env['OS_GROUP'] == 'windows':
    vars = Variables()
    vars.Add(PathVariable('OPENSSL_BASE', 'Base OpenSSL directory (windows only)', os.environ.get('OPENSSL_BASE')))
    vars.Update(env)
    Help(vars.GenerateHelpText(env))
    env.AppendUnique(LIBS = ['setupapi', 'user32', 'winmm', 'ws2_32', 'iphlpapi', 'secur32', 'Advapi32'])
    # Key of presence of OPENSSL_BASE to decide if to use openssl or window CNG crypto
    if '' == env.subst('$OPENSSL_BASE'):
        if env['OS'] == 'winxp':
            # Must specify OPENSSL_BASE for winXP
            print 'Must specify OPENSSL_BASE when building for WindowsXP'
            if not GetOption('help'):
                Exit()
        else:
            env.AppendUnique(LIBS = ['bcrypt', 'ncrypt', 'crypt32'])
            env['CRYPTO'] = 'cng'
            print 'Using CNG crypto libraries'
    else:
        env.Append(CPPPATH = ['$OPENSSL_BASE/include'])
        env.Append(LIBPATH = ['$OPENSSL_BASE/lib'])
        env.AppendUnique(LIBS = ['libeay32', 'ssleay32'])
        print 'Using OPENSSL crypto libraries'

elif env['OS_GROUP'] == 'winrt':
    env['CRYPTO'] = 'winrt'
    print 'Using WINRT crypto libraries'
    env.AppendUnique(CFLAGS=['/D_WINRT_DLL'])
    env.AppendUnique(CXXFLAGS=['/D_WINRT_DLL'])	

elif env['OS'] in ['linux', 'openwrt']:
    env.AppendUnique(LIBS =['rt', 'stdc++', 'pthread', 'crypto', 'ssl', 'm'])

elif env['OS'] == 'darwin':
    env.AppendUnique(LIBS =['stdc++', 'pthread', 'crypto', 'ssl'])
    if env['CPU'] in ['arm', 'armv7', 'armv7s']:
        vars = Variables()
        vars.Add(PathVariable('OPENSSL_ROOT', 'Base OpenSSL directory (darwin only)', os.environ.get('OPENSSL_ROOT')))
        vars.Update(env)
        Help(vars.GenerateHelpText(env))
        if '' == env.subst('$OPENSSL_ROOT'):
            # Must specify OPENSSL_ROOT for darwin, arm
            print 'Must specify OPENSSL_ROOT when building for OS=darwin, CPU=arm'
            if not GetOption('help'):
                Exit()
        env.Append(CPPPATH = ['$OPENSSL_ROOT/include'])
        env.Append(LIBPATH = ['$OPENSSL_ROOT/build/' + os.environ.get('CONFIGURATION') + '-' + os.environ.get('PLATFORM_NAME')])

elif env['OS'] == 'android':
    env.AppendUnique(LIBS = ['m', 'c', 'stdc++', 'crypto', 'log', 'gcc', 'ssl'])
    if env.subst('$ANDROID_NDK_VERSION') in ['7', '8', '8b', '8c', '8d']:
        env.AppendUnique(LIBS = ['gnustl_static'])


commonenv = env.Clone()

# Variant settings
commonenv.VariantDir('$OBJDIR', 'src', duplicate = 0)
commonenv.VariantDir('$OBJDIR/os', 'os/${OS_GROUP}', duplicate = 0)
commonenv.VariantDir('$OBJDIR/crypto', 'crypto/${CRYPTO}', duplicate = 0)

# Setup dependent include directories
if commonenv['OS_GROUP'] == 'winrt':
    hdrs = {}
else:
    hdrs = { 'qcc': commonenv.File(['inc/qcc/Log.h',
                                    'inc/qcc/Debug.h',
                                    'inc/qcc/ManagedObj.h',
                                    'inc/qcc/String.h',
                                    'inc/qcc/StringUtil.h',
                                    'inc/qcc/atomic.h',
                                    'inc/qcc/platform.h']),
             'qcc/${OS_GROUP}': commonenv.File(['inc/qcc/${OS_GROUP}/atomic.h',
                                                'inc/qcc/${OS_GROUP}/platform_types.h']) }

    if commonenv['OS_GROUP'] == 'windows':
        hdrs['qcc/${OS_GROUP}'] += commonenv.File(['inc/qcc/${OS_GROUP}/mapping.h'])

# Build the sources
status_cpp0x_src = ['Status_CPP0x.cc', 'StatusComment.cc']
status_src = ['Status.cc']

srcs = commonenv.Glob('$OBJDIR/*.cc') + commonenv.Glob('$OBJDIR/os/*.cc') + commonenv.Glob('$OBJDIR/crypto/*.cc')

if commonenv['OS_GROUP'] == 'winrt':
    srcs = [ f for f in srcs if basename(str(f)) not in status_cpp0x_src ]

# Make sure Status never gets included from common for contained projects
srcs = [ f for f in srcs if basename(str(f)) not in status_src ]

static_objs = env.Object(srcs)

if commonenv['LIBTYPE'] != 'static':
    shared_objs = commonenv.SharedObject(srcs)
else:
    shared_objs = []

# under normal build conditions the Status.xml found in alljoyn_core is used to
# build Status.h and Status.cc.  If we are building the code in common independent
# of the alljoyn_core we will have to create Status.h and Status.cc for common.
status_obj = [];
if commonenv.has_key('BUILD_COMMON_STATUS'):
    commonenv.Install('$OBJDIR', commonenv.File('src/Status.xml'))
    status_src, status_hdr = commonenv.Status('$OBJDIR/Status')
    status_obj = commonenv.Object(status_src)
    commonenv.Append(CPPPATH = ['#' + os.path.dirname(str(status_hdr))])

else:
    # allow common to "#include <Status.h>" when building all of AllJoyn
    commonenv.Append(CPPPATH = ['$DISTDIR/cpp/inc/alljoyn'])

libcommon = commonenv.StaticLibrary('$OBJDIR/common_static', [static_objs, status_obj])

# Build unit Tests
commonenv.SConscript('unit_test/SConscript', variant_dir='$OBJDIR/unittest', duplicate=0, exports=['libcommon'])

ret = (hdrs, static_objs, shared_objs)

Return('ret')
