#pragma once

// Silly windows macros!

#if defined(LoadString)
#undef LoadString
#endif

#if defined(GetObject)
#undef GetObject
#endif

#if defined(MessageBox)
#undef MessageBox                       // Redefines Urho3D::MessageBox
#endif
#if defined(GetMessage)
#undef GetMessage
#endif

#if defined(GetClassName)
#undef GetClassName
#endif

#if defined(GetFreeSpace)
#undef GetFreeSpace
#endif
