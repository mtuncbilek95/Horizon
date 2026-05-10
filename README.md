# Horizon Engine

It's mediator based, context and system separated rendering engine.

### Context
Data holder that may be needed as a global accessor. Engine holds them and every system or context can access required contexts
through engine.

### System
Main control subjects of the engine. It means that they may need to do some main thread work at some point. E.g WindowSystem
needs to call ProcessEvents() on each while loop.