#include <OpenEXR/ImfInputFile.h>
#include <OpenEXR/ImfHeader.h>
#include <OpenEXR/ImfChannelList.h>
#include <OpenEXR/ImfIO.h>

#include <vector>
#include <map>
#include <string>
#include <cstdint>
#include <algorithm>
#include <stdexcept>

#include <emscripten.h>
#include <emscripten/bind.h>
#include <emscripten/val.h>

class BufferAdapter : public OPENEXR_IMF_NAMESPACE::IStream
{
public:
	BufferAdapter(char * buffer, size_t size) :
		IStream("BufferAdapter"),
		m_buffer(buffer), m_size(size), m_offset()
	{
	}

	virtual bool isMemoryMapped() const override
	{
		return true;
	}

	virtual bool read(char c[/*n*/], int n) override
	{
		std::size_t remaining = m_size - m_offset;
		if (std::size_t(n) > remaining)
			throw std::runtime_error("Out of bounds read request");
		auto current = readMemoryMapped(n);
		std::copy(current, current + n, c);
		return n == remaining;
	}

	virtual char * readMemoryMapped(int n) override
	{
		std::size_t remaining = m_size - m_offset;
		if (std::size_t(n) > remaining)
			throw std::runtime_error("Out of bounds read request");
		auto current = bufferCurrent();
		m_offset += n;
		return current;
	}

	virtual OPENEXR_IMF_NAMESPACE::Int64 tellg() override
	{
		return m_offset;
	}

	virtual void seekg (OPENEXR_IMF_NAMESPACE::Int64 pos) override
	{
		m_offset = pos;
	}

public:
	char * bufferCurrent() const
	{
		return m_buffer + m_offset;
	}

private:
	char * m_buffer;
	std::size_t m_size;
	std::size_t m_offset;
};

typedef std::vector<char> Bytes;
typedef std::vector<float> Pixel;
typedef std::map<std::string, Pixel> Planes;

struct EXRImage
{
	std::size_t width;
	std::size_t height;
	Planes planes;

	emscripten::val plane(std::string const & name) const
	{
		using namespace emscripten;
		auto it = planes.find(name);
		if (it == planes.end())
			return val::undefined();
		return val(typed_memory_view(it->second.size(), it->second.data()));
	}
	
	emscripten::val channels() const
	{
		using namespace emscripten;
		auto c = val::array();
		std::size_t idx = 0;
		for (auto const & plane : planes)
			c.set(idx++, plane.first);
		return val(c);
	}
};

EXRImage loadEXRRaw(char const * buffer, std::size_t size)
{
	using namespace OPENEXR_IMF_NAMESPACE;
	EXRImage image;
	try {
		BufferAdapter bytes(const_cast<char*>(buffer), size);
		InputFile file(bytes);
		auto window = file.header().dataWindow();
		image.width = window.max.x - window.min.x + 1;
		image.height = window.max.y - window.min.y + 1;
		size_t strideX = sizeof(float);
		size_t strideY = sizeof(float) * image.width;
		FrameBuffer fb;
		auto const & channels = file.header().channels();
		auto it = channels.begin();
		auto ed = channels.end();
		for (; it != ed; ++it) // OpenEXR iterators are not real iterators so ranged for loop doesn't work
		{
			auto const & name = it.name();
			image.planes[name].resize(image.width * image.height);
			fb.insert(name, Slice(FLOAT, (char*)image.planes[name].data(), strideX, strideY));
		}
		file.setFrameBuffer(fb);
		file.readPixels(window.min.y, window.max.y);
	}
	catch (...)
	{
	}
	return image;
}

EXRImage loadEXRVec(std::vector<char> const & bytes)
{
	return loadEXRRaw(bytes.data(), bytes.size());
}

EXRImage loadEXRStr(std::string const & bytes)
{
	
	return loadEXRRaw(bytes.data(), bytes.size());
}

#if defined(WRAPPER_EMBIND)

// http://kripken.github.io/emscripten-site/docs/porting/connecting_cpp_and_javascript/embind.html
EMSCRIPTEN_BINDINGS(Wrapper)
{
	using namespace emscripten;
	register_vector<float>("Pixel");
	register_vector<char>("Bytes");
	register_map<std::string, Pixel>("Planes");
	class_<EXRImage>("EXRImage")
		.property("width", &EXRImage::width)
		.property("height", &EXRImage::height)
		.property("planes", &EXRImage::planes)
		.function("plane", &EXRImage::plane)
		.function("channels", &EXRImage::channels)
	;
	function("loadEXRRaw", &loadEXRRaw, allow_raw_pointer<arg<0>>());
	function("loadEXRVec", &loadEXRVec);
	function("loadEXRStr", &loadEXRStr);
}

#endif
