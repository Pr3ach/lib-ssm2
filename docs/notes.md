## General
+ Block read query seems to handle no more than 0xf9 as a range which is 0xf9+1 = 250 bytes max. in return

## SSM2 internals
+ Block read vs Addr read: the first 255 bytes are different (for same addr request), then it looks the same
+ Block read seems to start @ 0x100000 even if specified start addr is lower than that (e.g 0x00)
  => Looks like SSM2 is not working with direct hardware addresses, rather
  PID-like shit => ECU seems to use like a lookup-table to translate requested addresses to valid ones (self + legacygt forums)
+ ROM data seems to start @ 0x100000 with length 0x020000 (Sasha_A80 @ romraider forums)
