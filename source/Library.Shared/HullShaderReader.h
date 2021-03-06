#pragma once

#include "ContentTypeReader.h"
#include "HullShader.h"

namespace Library
{
	class HullShaderReader : public ContentTypeReader<HullShader>
	{
		RTTI_DECLARATIONS(HullShaderReader, AbstractContentTypeReader)

	public:
		HullShaderReader(Game& game);
		HullShaderReader(const HullShaderReader&) = default;
		HullShaderReader& operator=(const HullShaderReader&) = default;
		HullShaderReader(HullShaderReader&&) = default;
		HullShaderReader& operator=(HullShaderReader&&) = default;
		~HullShaderReader() = default;

	protected:
		virtual std::shared_ptr<HullShader> _Read(const std::wstring& assetName) override;
	};
}