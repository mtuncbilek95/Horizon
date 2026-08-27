"""Dumps a cooked Horizon scene (.hfile) to the terminal, entity by entity.

The scene is a BinaryArchive (Runtime/Serialization/BinaryArchive.h): keys are stored as
FNV-1a hashes, so readable names come from the generated reflection headers under
Intermediate/ plus the HFIELD() declarations they point at.

    python SceneDumper.py ExampleProject/Cooked/TestWorld.hfile
    python SceneDumper.py TestWorld.hfile --entity 2 --chunks
"""

import argparse
import os
import re
import struct
import sys

ARCHIVE_MAGIC = 0x4E5A5248
ARCHIVE_VERSION = 1
BLOB_ALIGNMENT = 16

SCENE_FORMAT_VERSION = 1
CHUNK_FLAG_BLITTABLE = 1 << 0
CHUNK_FLAG_NEEDS_FIXUP = 1 << 1

SCENE_KEYS = ["version", "typeNames", "dependencies", "entityCount", "chunks",
              "typeIndex", "count", "stride", "flags", "entities", "objects", "data", "type"]

INT_TYPES = {"c8": 1, "i8": 1, "i16": 2, "i32": 4, "i64": 8}
UINT_TYPES = {"u8": 1, "u16": 2, "u32": 4, "u64": 8, "usize": 8}
FLOAT_TYPES = {"f32": 4, "float": 4, "f64": 8, "double": 8}
BOOL_TYPES = {"b8": 1, "bool": 1}
TEXT_TYPES = {"std::string": 32, "string": 32, "Guid": 16, "PAL::DateTime": 8, "DateTime": 8}

POINTER_SIZE = 8
VTABLE_SIZE = 8


def Fnv1a(name):
	"""Mirrors HashArchiveName in BinaryArchive.cpp."""
	hashValue = 2166136261

	for byte in name.encode("utf-8"):
		hashValue ^= byte
		hashValue = (hashValue * 16777619) & 0xFFFFFFFF

	return hashValue


class DumpError(Exception):
	pass


class FieldDef:
	"""One reflected field: the archive name, the C++ declaration and how to read it."""

	def __init__(self, name, member, cppType):
		self.name = name
		self.member = member
		self.cppType = cppType
		self.hash = Fnv1a(name)
		self.mode = "value"
		self.element = cppType

		if cppType is None:
			self.kind = "unknown"
			return

		text = cppType.strip()

		if text.endswith("*"):
			self.mode = "pointer"
			self.element = text[:-1].strip()
			self.kind = "object"
			return

		listMatch = re.match(r"^(?:Horizon::)?List<(.+)>$", text)

		if listMatch:
			self.mode = "array"
			self.element = listMatch.group(1).strip()
			self.kind = KindOf(self.element)
			return

		self.kind = KindOf(text)

	def __repr__(self):
		return f"FieldDef({self.name}, {self.cppType}, {self.kind}, {self.mode})"


def StripNamespace(cppType):
	return cppType.split("::")[-1]


def KindOf(cppType):
	text = cppType.strip()
	bare = StripNamespace(text)

	if text in BOOL_TYPES or bare in BOOL_TYPES:
		return "bool"

	if text in INT_TYPES or bare in INT_TYPES:
		return "int"

	if text in UINT_TYPES or bare in UINT_TYPES:
		return "uint"

	if text in FLOAT_TYPES or bare in FLOAT_TYPES:
		return "float"

	if text in TEXT_TYPES or bare in ("string", "Guid", "DateTime"):
		return "string"

	return "object"


def SizeOfScalar(cppType):
	bare = StripNamespace(cppType.strip())

	for table in (BOOL_TYPES, INT_TYPES, UINT_TYPES, FLOAT_TYPES):
		if bare in table:
			return table[bare]

	return None


class TypeDef:
	"""A reflected type with its own fields, in declaration order."""

	def __init__(self, name, base, fields):
		self.name = name
		self.base = base
		self.fields = fields
		self.byHash = {field.hash: field for field in fields}


class Schema:
	"""Every reflected type found under Intermediate/, keyed by its reflected name."""

	def __init__(self):
		self.types = {}
		self.names = dict((Fnv1a(key), key) for key in SCENE_KEYS)
		self.warnings = []

	def Get(self, typeName):
		return self.types.get(StripNamespace(typeName.strip()))

	def NameOf(self, hashValue):
		return self.names.get(hashValue)

	def Add(self, typeDef):
		self.types[typeDef.name] = typeDef

		for field in typeDef.fields:
			self.names[field.hash] = field.name


REFLECT_TYPE_PATTERN = re.compile(r'ForType\("(\w+)"\)')
REFLECT_BASE_PATTERN = re.compile(r"WithBase<\s*([\w:]+)\s*>")
REFLECT_FIELD_PATTERN = re.compile(r'WithField\(\s*"(\w+)"\s*,\s*&[\w:]+::(\w+)\s*\)')
REFLECT_INCLUDE_PATTERN = re.compile(r"#include\s+<(.+?)>")
MEMBER_PATTERN = re.compile(r"^\s*([A-Za-z_][\w:<>,\s\*]*?)\s+(\w+)\s*(?:=[^;]*)?;")


def ParseMembers(headerPath):
	"""Maps member -> C++ type for every HFIELD() declaration in a source header."""
	members = {}

	try:
		with open(headerPath, "r", encoding="utf-8", errors="replace") as handle:
			lines = handle.readlines()
	except OSError:
		return members

	for index, line in enumerate(lines):
		if "HFIELD(" not in line:
			continue

		for candidate in lines[index + 1:index + 4]:
			match = MEMBER_PATTERN.match(candidate)

			if not match:
				continue

			cppType = " ".join(match.group(1).split())
			members[match.group(2)] = cppType
			break

	return members


def LoadSchema(reflectionDir, sourceDir):
	schema = Schema()

	if not reflectionDir or not os.path.isdir(reflectionDir):
		schema.warnings.append(f"reflection directory '{reflectionDir}' not found, names fall back to hashes")
		return schema

	memberCache = {}

	for root, _, files in os.walk(reflectionDir):
		for fileName in files:
			if not fileName.endswith(".reflected.h"):
				continue

			path = os.path.join(root, fileName)

			with open(path, "r", encoding="utf-8", errors="replace") as handle:
				text = handle.read()

			typeMatch = REFLECT_TYPE_PATTERN.search(text)

			if not typeMatch:
				continue

			baseMatch = REFLECT_BASE_PATTERN.search(text)
			includeMatch = REFLECT_INCLUDE_PATTERN.search(text)

			members = {}

			if includeMatch and sourceDir:
				headerPath = os.path.join(sourceDir, includeMatch.group(1).replace("/", os.sep))

				if headerPath not in memberCache:
					memberCache[headerPath] = ParseMembers(headerPath)

				members = memberCache[headerPath]

			fields = []

			for fieldName, member in REFLECT_FIELD_PATTERN.findall(text):
				cppType = members.get(member)

				if cppType is None:
					schema.warnings.append(f"{typeMatch.group(1)}::{member} has no HFIELD declaration, value is guessed")

				fields.append(FieldDef(fieldName, member, cppType))

			schema.Add(TypeDef(typeMatch.group(1), baseMatch.group(1) if baseMatch else None, fields))

	return schema


class Archive:
	"""Random access reader over a BinaryArchive buffer."""

	def __init__(self, data):
		if len(data) < 8:
			raise DumpError("file is smaller than the archive header")

		magic, version = struct.unpack_from("<II", data, 0)

		if magic != ARCHIVE_MAGIC:
			raise DumpError(f"magic {magic:#010x} is not a Horizon archive")

		if version > ARCHIVE_VERSION:
			raise DumpError(f"archive version {version} is newer than the supported {ARCHIVE_VERSION}")

		self.data = data
		self.version = version
		self.rootOffset = 8

	def U32(self, offset):
		self.Check(offset, 4)
		return struct.unpack_from("<I", self.data, offset)[0]

	def Check(self, offset, size):
		if offset < 0 or offset + size > len(self.data):
			raise DumpError(f"read of {size} bytes at {offset} runs past the archive")

	def ObjectEntries(self, offset):
		"""Returns (entries, endOffset) where an entry is (hash, payloadOffset, payloadSize)."""
		payloadSize, fieldCount = struct.unpack_from("<II", self.data, offset)
		self.Check(offset + 8, payloadSize)

		cursor = offset + 8
		entries = []

		for _ in range(fieldCount):
			keyHash, entrySize = struct.unpack_from("<II", self.data, cursor)
			entries.append((keyHash, cursor + 8, entrySize))
			cursor = cursor + 8 + entrySize

		return entries, offset + 8 + payloadSize

	def ArrayHeader(self, offset):
		"""Returns (elementCount, payloadOffset, endOffset)."""
		payloadSize, elementCount = struct.unpack_from("<II", self.data, offset)
		self.Check(offset + 8, payloadSize)

		return elementCount, offset + 8, offset + 8 + payloadSize

	def ReadString(self, offset):
		length = self.U32(offset)
		self.Check(offset + 4, length)

		return self.data[offset + 4:offset + 4 + length].decode("utf-8", "replace"), offset + 4 + length

	def ReadBlob(self, offset):
		"""Returns (dataOffset, size, endOffset), honouring the 16 byte payload alignment."""
		size = self.U32(offset)
		cursor = offset + 4
		remainder = cursor % BLOB_ALIGNMENT

		if remainder != 0:
			cursor += BLOB_ALIGNMENT - remainder

		self.Check(cursor, size)
		return cursor, size, cursor + size


class ObjectValue:
	def __init__(self, typeName, pairs):
		self.typeName = typeName
		self.pairs = pairs


class RawValue:
	def __init__(self, data, note):
		self.data = data
		self.note = note


class Dumper:
	def __init__(self, archive, schema):
		self.archive = archive
		self.schema = schema
		self.notes = []

	def Note(self, text):
		if text not in self.notes:
			self.notes.append(text)

	def ReadObject(self, offset, typeDef):
		entries, endOffset = self.archive.ObjectEntries(offset)
		pairs = []

		for keyHash, payloadOffset, payloadSize in entries:
			field = typeDef.byHash.get(keyHash) if typeDef else None
			name = field.name if field else self.schema.NameOf(keyHash) or f"#{keyHash:08x}"

			if field and field.kind != "unknown":
				value, _ = self.ReadField(payloadOffset, field)
			else:
				value = self.GuessValue(payloadOffset, payloadSize)

			pairs.append((name, value))

		return ObjectValue(typeDef.name if typeDef else None, pairs), endOffset

	def ReadField(self, offset, field):
		if field.mode == "pointer":
			return self.ReadPointer(offset, field)

		if field.mode == "array":
			elementCount, cursor, endOffset = self.archive.ArrayHeader(offset)
			values = []

			for _ in range(elementCount):
				value, cursor = self.ReadValue(cursor, field)
				values.append(value)

			return values, endOffset

		return self.ReadValue(offset, field)

	def ReadPointer(self, offset, field):
		entries, endOffset = self.archive.ObjectEntries(offset)
		typeName = None
		value = None

		for keyHash, payloadOffset, payloadSize in entries:
			if keyHash == Fnv1a("type"):
				typeName, _ = self.archive.ReadString(payloadOffset)
			elif keyHash == Fnv1a("data"):
				value, _ = self.ReadObject(payloadOffset, self.schema.Get(typeName) if typeName else None)

		if value is None:
			return ObjectValue(typeName, []), endOffset

		value.typeName = typeName
		return value, endOffset

	def ReadValue(self, offset, field):
		data = self.archive.data

		if field.kind == "bool":
			self.archive.Check(offset, 1)
			return data[offset] != 0, offset + 1

		if field.kind == "int":
			self.archive.Check(offset, 8)
			return struct.unpack_from("<q", data, offset)[0], offset + 8

		if field.kind == "uint":
			self.archive.Check(offset, 8)
			return struct.unpack_from("<Q", data, offset)[0], offset + 8

		if field.kind == "float":
			self.archive.Check(offset, 8)
			return struct.unpack_from("<d", data, offset)[0], offset + 8

		if field.kind == "string":
			return self.archive.ReadString(offset)

		nested = self.schema.Get(field.element)

		if not nested:
			self.Note(f"'{field.element}' is not reflected, its fields are guessed")

		return self.ReadObject(offset, nested)

	def GuessValue(self, offset, size):
		"""Last resort for entries with no reflected field behind them."""
		data = self.archive.data
		self.archive.Check(offset, size)

		if size == 1:
			return data[offset] != 0

		if size >= 8:
			payloadSize = struct.unpack_from("<I", data, offset)[0]

			if payloadSize + 8 == size:
				try:
					value, _ = self.ReadObject(offset, None)
					return value
				except (DumpError, struct.error):
					pass

		if size >= 4:
			length = struct.unpack_from("<I", data, offset)[0]

			if length + 4 == size:
				return self.archive.ReadString(offset)[0]

		if size == 8:
			asFloat = struct.unpack_from("<d", data, offset)[0]
			asInt = struct.unpack_from("<q", data, offset)[0]
			return RawValue(data[offset:offset + size], f"i64={asInt} f64={asFloat:.6g}")

		return RawValue(data[offset:offset + size], f"{size} bytes")


class Chunk:
	def __init__(self):
		self.typeIndex = 0xFFFFFFFF
		self.count = 0
		self.stride = 0
		self.flags = 0
		self.entities = []
		self.dataOffset = 0
		self.dataSize = 0
		self.typeName = "<unknown>"


class Scene:
	def __init__(self):
		self.version = 0
		self.typeNames = []
		self.dependencies = []
		self.entityCount = 0
		self.chunks = []


def ParseScene(archive, dumper):
	scene = Scene()
	entries, _ = archive.ObjectEntries(archive.rootOffset)

	for keyHash, offset, size in entries:
		if keyHash == Fnv1a("version"):
			scene.version = struct.unpack_from("<Q", archive.data, offset)[0]
		elif keyHash == Fnv1a("entityCount"):
			scene.entityCount = struct.unpack_from("<Q", archive.data, offset)[0]
		elif keyHash == Fnv1a("typeNames"):
			count, cursor, _ = archive.ArrayHeader(offset)

			for _ in range(count):
				value, cursor = archive.ReadString(cursor)
				scene.typeNames.append(value)
		elif keyHash == Fnv1a("dependencies"):
			count, cursor, _ = archive.ArrayHeader(offset)

			for _ in range(count):
				value, cursor = archive.ReadString(cursor)
				scene.dependencies.append(value)
		elif keyHash == Fnv1a("chunks"):
			count, cursor, _ = archive.ArrayHeader(offset)

			for _ in range(count):
				chunk, cursor = ParseChunk(archive, cursor)
				scene.chunks.append(chunk)

	if scene.version > SCENE_FORMAT_VERSION:
		dumper.Note(f"scene version {scene.version} is newer than the supported {SCENE_FORMAT_VERSION}")

	for chunk in scene.chunks:
		if chunk.typeIndex < len(scene.typeNames):
			chunk.typeName = scene.typeNames[chunk.typeIndex]
		else:
			dumper.Note(f"a chunk names the unknown type index {chunk.typeIndex}")

	return scene


def ParseChunk(archive, offset):
	chunk = Chunk()
	entries, endOffset = archive.ObjectEntries(offset)

	for keyHash, payloadOffset, payloadSize in entries:
		if keyHash == Fnv1a("typeIndex"):
			chunk.typeIndex = struct.unpack_from("<Q", archive.data, payloadOffset)[0]
		elif keyHash == Fnv1a("count"):
			chunk.count = struct.unpack_from("<Q", archive.data, payloadOffset)[0]
		elif keyHash == Fnv1a("stride"):
			chunk.stride = struct.unpack_from("<Q", archive.data, payloadOffset)[0]
		elif keyHash == Fnv1a("flags"):
			chunk.flags = struct.unpack_from("<Q", archive.data, payloadOffset)[0]
		elif keyHash == Fnv1a("entities"):
			dataOffset, size, _ = archive.ReadBlob(payloadOffset)
			chunk.entities = list(struct.unpack_from(f"<{size // 4}I", archive.data, dataOffset))
		elif keyHash == Fnv1a("data"):
			chunk.dataOffset, chunk.dataSize, _ = archive.ReadBlob(payloadOffset)
		elif keyHash == Fnv1a("objects"):
			chunk.dataOffset = payloadOffset
			chunk.dataSize = payloadSize

	return chunk, endOffset


def BuildLayout(typeDef, schema):
	"""Best effort C++ layout for a blittable chunk: vtable pointer, then fields in order."""
	offset = VTABLE_SIZE if typeDef.base else 0
	alignment = 8 if typeDef.base else 1
	entries = []

	for field in typeDef.fields:
		if field.mode == "pointer":
			size, memberAlign = POINTER_SIZE, POINTER_SIZE
		elif field.mode == "array" or field.kind in ("string", "unknown"):
			return None
		elif field.kind == "object":
			nested = schema.Get(field.element)

			if not nested:
				return None

			nestedLayout = BuildLayout(nested, schema)

			if not nestedLayout:
				return None

			size, memberAlign = nestedLayout[1], nestedLayout[2]
		else:
			size = SizeOfScalar(field.cppType)

			if not size:
				return None

			memberAlign = size

		remainder = offset % memberAlign

		if remainder != 0:
			offset += memberAlign - remainder

		entries.append((field, offset))
		alignment = max(alignment, memberAlign)
		offset += size

	remainder = offset % alignment

	if remainder != 0:
		offset += alignment - remainder

	return entries, offset, alignment


def ReadBlittable(archive, schema, typeDef, offset):
	layout = BuildLayout(typeDef, schema)

	if not layout:
		return None

	pairs = []

	for field, fieldOffset in layout[0]:
		cursor = offset + fieldOffset

		if field.kind == "object":
			nested = schema.Get(field.element)
			value = ReadBlittable(archive, schema, nested, cursor)
		else:
			value = ReadPlain(archive.data, cursor, field.cppType, field.kind)

		pairs.append((field.name, value))

	return ObjectValue(typeDef.name, pairs)


def ReadPlain(data, offset, cppType, kind):
	size = SizeOfScalar(cppType) or 8
	formats = {("bool", 1): "<?", ("int", 1): "<b", ("int", 2): "<h", ("int", 4): "<i", ("int", 8): "<q",
	           ("uint", 1): "<B", ("uint", 2): "<H", ("uint", 4): "<I", ("uint", 8): "<Q",
	           ("float", 4): "<f", ("float", 8): "<d"}
	form = formats.get((kind, size))

	if not form:
		return RawValue(data[offset:offset + size], f"{size} bytes")

	return struct.unpack_from(form, data, offset)[0]


def ReadChunkValues(archive, dumper, schema, chunk):
	"""Returns one decoded value per element of the chunk."""
	typeDef = schema.Get(chunk.typeName)

	if not typeDef:
		dumper.Note(f"'{chunk.typeName}' is not reflected in this tree, its fields are guessed")

	values = []

	if chunk.flags & CHUNK_FLAG_BLITTABLE:
		if not typeDef or chunk.count == 0:
			return [RawValue(b"", "blittable payload, type unknown")] * chunk.count

		stride = chunk.stride or (chunk.dataSize // chunk.count)
		layout = BuildLayout(typeDef, schema)

		if not layout or layout[1] != stride:
			computed = layout[1] if layout else "unknown"
			dumper.Note(f"'{chunk.typeName}' is {stride} bytes on disk but the layout here computes {computed}, raw bytes shown")

			for index in range(chunk.count):
				start = chunk.dataOffset + index * stride
				values.append(RawValue(archive.data[start:start + stride], f"{stride} raw bytes"))

			return values

		for index in range(chunk.count):
			values.append(ReadBlittable(archive, schema, typeDef, chunk.dataOffset + index * stride))

		return values

	elementCount, cursor, _ = archive.ArrayHeader(chunk.dataOffset)

	for _ in range(min(elementCount, chunk.count)):
		value, cursor = dumper.ReadObject(cursor, typeDef)
		values.append(value)

	if elementCount != chunk.count:
		dumper.Note(f"chunk '{chunk.typeName}' claims {chunk.count} elements but carries {elementCount}")

	return values


def FormatScalar(value):
	if isinstance(value, bool):
		return "true" if value else "false"

	if isinstance(value, float):
		return f"{value:.6g}"

	if isinstance(value, str):
		return f'"{value}"'

	if isinstance(value, RawValue):
		return f"<{value.note}: {value.data[:16].hex()}>"

	return str(value)


def IsCompact(value):
	if isinstance(value, ObjectValue):
		return len(value.pairs) <= 4 and all(not isinstance(item, (ObjectValue, list)) for _, item in value.pairs)

	return not isinstance(value, list)


VECTOR_FIELDS = (["X", "Y"], ["X", "Y", "Z"], ["X", "Y", "Z", "W"])


def FormatCompact(value):
	if isinstance(value, ObjectValue):
		names = [name for name, _ in value.pairs]

		if names in VECTOR_FIELDS:
			return "(" + ", ".join(FormatScalar(item) for _, item in value.pairs) + ")"

		body = ", ".join(f"{name} = {FormatScalar(item)}" for name, item in value.pairs)
		return "{ " + body + " }" if body else "{ }"

	return FormatScalar(value)


def PrintValue(name, value, indent, out):
	pad = " " * indent

	if IsCompact(value):
		out.append(f"{pad}{name} = {FormatCompact(value)}")
		return

	if isinstance(value, list):
		out.append(f"{pad}{name} = [{len(value)}]")

		for index, item in enumerate(value):
			PrintValue(f"[{index}]", item, indent + 2, out)

		return

	out.append(f"{pad}{name} =")

	for childName, child in value.pairs:
		PrintValue(childName, child, indent + 2, out)


def Dump(path, entityFilter, showChunks, schema, out):
	with open(path, "rb") as handle:
		data = handle.read()

	archive = Archive(data)
	dumper = Dumper(archive, schema)
	scene = ParseScene(archive, dumper)

	perEntity = [[] for _ in range(scene.entityCount)]
	strays = []

	for chunk in scene.chunks:
		values = ReadChunkValues(archive, dumper, schema, chunk)

		for index, value in enumerate(values):
			entity = chunk.entities[index] if index < len(chunk.entities) else None

			if entity is None or entity >= scene.entityCount:
				strays.append((chunk.typeName, entity, value))
				continue

			perEntity[entity].append((chunk.typeName, value))

	out.append("=" * 78)
	out.append(f" Scene        : {path}")
	out.append(f" Size         : {len(data)} bytes, archive version {archive.version}")
	out.append(f" Scene version: {scene.version}")
	out.append(f" Entities     : {scene.entityCount}")
	out.append(f" Chunks       : {len(scene.chunks)}")
	out.append(f" Dependencies : {', '.join(scene.dependencies) if scene.dependencies else 'none'}")
	out.append("=" * 78)

	out.append("")
	out.append("Component types")

	for index, typeName in enumerate(scene.typeNames):
		total = sum(chunk.count for chunk in scene.chunks if chunk.typeIndex == index)
		known = "reflected" if schema.Get(typeName) else "not reflected here"
		out.append(f"  [{index}] {typeName:<32} {total:>4} instance(s)   {known}")

	if showChunks:
		out.append("")
		out.append("Chunks")

		for index, chunk in enumerate(scene.chunks):
			names = []

			if chunk.flags & CHUNK_FLAG_BLITTABLE:
				names.append("blittable")

			if chunk.flags & CHUNK_FLAG_NEEDS_FIXUP:
				names.append("needsFixup")

			flags = ", ".join(names) if names else "none"
			out.append(f"  [{index}] {chunk.typeName:<32} count={chunk.count} stride={chunk.stride} flags={flags}")
			out.append(f"       entities={chunk.entities} data@{chunk.dataOffset} ({chunk.dataSize} bytes)")

	for entity in range(scene.entityCount):
		if entityFilter and entity not in entityFilter:
			continue

		components = perEntity[entity]
		label = EntityLabel(components)

		out.append("")
		out.append(f"Entity {entity}{label} — {len(components)} component(s)")

		if not components:
			out.append("  <no components>")

		for typeName, value in components:
			out.append(f"  {typeName}")

			if isinstance(value, ObjectValue) and not value.pairs:
				out.append("    <no fields>")
				continue

			if isinstance(value, ObjectValue):
				for fieldName, fieldValue in value.pairs:
					PrintValue(fieldName, fieldValue, 4, out)
			else:
				PrintValue("value", value, 4, out)

	if strays:
		out.append("")
		out.append("Components bound to entities outside the scene")

		for typeName, entity, _ in strays:
			out.append(f"  {typeName} -> entity {entity}")

	notes = dumper.notes + schema.warnings

	if notes:
		out.append("")
		out.append("Notes")

		for note in notes:
			out.append(f"  - {note}")


def EntityLabel(components):
	for typeName, value in components:
		if typeName != "NameComponent" or not isinstance(value, ObjectValue):
			continue

		for fieldName, fieldValue in value.pairs:
			if isinstance(fieldValue, str):
				return f' "{fieldValue}"'

	return ""


def Main(argv):
	repoRoot = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

	parser = argparse.ArgumentParser(description="Dumps a cooked Horizon scene to the terminal")
	parser.add_argument("scene", nargs="?", default=os.path.join(repoRoot, "ExampleProject", "Cooked", "TestWorld.hfile"),
	                    help="path to the .hfile scene")
	parser.add_argument("--reflection", default=os.path.join(repoRoot, "Intermediate"),
	                    help="directory holding the generated *.reflected.h files")
	parser.add_argument("--source", default=os.path.join(repoRoot, "Source"),
	                    help="source root the reflected headers include from")
	parser.add_argument("--entity", type=int, action="append", help="dump only this entity, repeatable")
	parser.add_argument("--chunks", action="store_true", help="also print the raw chunk table")
	args = parser.parse_args(argv)

	if not os.path.isfile(args.scene):
		print(f"SceneDumper: '{args.scene}' does not exist", file=sys.stderr)
		return 1

	schema = LoadSchema(args.reflection, args.source)
	out = []

	try:
		Dump(args.scene, set(args.entity or []), args.chunks, schema, out)
	except (DumpError, struct.error) as error:
		print("\n".join(out))
		print(f"SceneDumper: {error}", file=sys.stderr)
		return 1

	print("\n".join(out))
	return 0


if __name__ == "__main__":
	sys.exit(Main(sys.argv[1:]))
