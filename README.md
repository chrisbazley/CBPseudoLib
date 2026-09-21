# CBPseudoLib

CBPseudoLib provides fault-injecting and state-recording substitutes for
selected ISO C, Acorn C library, Flex, Wimp, Toolbox and event-library
interfaces. It is intended for tests built with Fortify.

The public header names are unchanged from their former home in CBDebugLib:

- `PseudoEvnt.h`
- `PseudoExit.h`
- `PseudoFlex.h`
- `PseudoIO.h`
- `PseudoKern.h`
- `PseudoTbox.h`
- `PseudoWimp.h`

CBPseudoLib depends on CBUtilLib, CBDebugLib, Fortify and OptionalAcornC.

The library can be built with CMake, GNU Make on Linux, GNU Make on RISC OS,
or Acorn Make Utility using `NMakefile`.

## History

- 21 September 2026: Extracted the pseudo interfaces from CBDebugLib into a
  dedicated library without changing their public include paths.

## Licence

CBPseudoLib is distributed under the GNU Lesser General Public License,
version 2.1 or later. See `LICENSE`.
