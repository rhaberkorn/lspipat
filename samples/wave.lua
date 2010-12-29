#!/usr/bin/lua

require "lspipat"

function uint(bytes, val) -- binary integer decoding
	return Len(bytes) % function(bin)
		bin = littleEndian and bin or bin:reverse()

		local n = 0
		local base = 1

		for _, c in ipairs{bin:byte(1, bytes)} do
			n = n + base * c
			base = base * 256
		end

		val(n)
	end
end

function _uint(bytes, name) return uint(bytes, function(n) _G[name] = n end) end
function _uint16(name) return _uint(2, name) end
function _uint32(name) return _uint(4, name) end

hnd = assert(io.open(arg[1]))

file = hnd:read("*a")

hnd:close()

-- WAVE file "grammar"

format = "fmt "
       * _uint32 "FmtChunkSize"
       * _Setcur "FmtStartPos"
       	 * _uint16 "AudioFormat"
	 * _uint16 "NumChannels"
	 * _uint32 "SampleRate"
	 * _uint32 "ByteRate"
	 * _uint16 "BlockAlign"
	 * _uint16 "BitsPerSample"
	 * ( -function() return AudioFormat == 1 end
	   + _uint16 "ExtraParamSize"
	   * _Len    "ExtraParamSize" )
       * Pos(function() return FmtStartPos + FmtChunkSize end)
       * -function() return BitsPerSample % 8 == 0 and
       			    BlockAlign == NumChannels * BitsPerSample/8 and
       			    ByteRate == SampleRate * BlockAlign end

data = "data"
     * _uint32 "DataChunkSize"
     * _Len    "DataChunkSize"

misc = Len(4)
     * _uint32 "MiscChunkSize"
     * _Len    "MiscChunkSize"

wave = (topattern("RIFF") + "RIFX")
     % function(id) littleEndian = id == "RIFF" end
     * _uint32 "ChunkSize"
     * _Setcur "StartPos"
       * "WAVE"
       * Arbno(format + data + misc)
     * Pos(function() return StartPos + ChunkSize end)
     * -function() return DataChunkSize % BlockAlign == 0 end
     * RPos(0)

assert(file:smatch(wave, spipat.match_anchored),
       arg[1].." is not a valid WAVE file!")

print(string.format(
"%s\
Format:		%u\
Channels:	%u\
Samplerate:	%u Hz\
Byterate:	%u Hz\
Bits/Sample:	%u\
Samples:	%u",
arg[1],
AudioFormat, NumChannels, SampleRate, ByteRate, BitsPerSample, DataChunkSize / BlockAlign))
print(os.date("Length:\t\t%T", DataChunkSize / ByteRate + 60*60*23))
