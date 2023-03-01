#pragma once

#define GUARD_BEGIN_NAMESPACE  namespace Kiran { namespace SessionGuard {
#define GUARD_END_NAMESPACE } }
#define GUARD_USEING_NAMESPACE using namespace ::Kiran::SessionGuard;

#define GUARD_GREETER_BEGIN_NAMESPACE namespace Kiran { namespace SessionGuard { namespace Greeter {
#define GUARD_GREETER_END_NAMESPACE } } }
#define GUARD_GREEETER_USING_NAMESPACE using namespace ::Kiran::SessionGuard::Greeter;

#define GUARD_LOCKER_BEGIN_NAMESPACE namespace Kiran { namespace SessionGuard { namespace Locker {
#define GUARD_LOCKER_END_NAMESPACE } } }
#define GUARD_LOCKER_USING_NMAESPACE using namespace ::Kiran::SessionGuard::Locker;

#define GUARD_POLKIT_AGENT_BEGIN_NAMESPACE namespace Kiran { namespace SessionGuard { namespace PolkitAgent {
#define GUARD_POLKIT_AGENT_END_NAMESPACE }}}
#define GUARD_POLKIT_AGENT_USING_NAMESPACE using namespace ::Kiran::SessionGuard::PolkitAgent;
