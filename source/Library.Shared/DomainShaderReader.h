#pragma once

#include "ContentTypeReader.h"
#include "DomainShader.h"

namespace Library
{
	class DomainShaderReader : public ContentTypeReader<DomainShader>
	{
		RTTI_DECLARATIONS(DomainShaderReader, AbstractContentTypeReader)

	public:
		DomainShaderReader(Game& game);
		DomainShaderReader(const DomainShaderReader&) = default;
		DomainShaderReader& operator=(const DomainShaderReader&) = default;
		DomainShaderReader(DomainShaderReader&&) = default;
		DomainShaderReader& operator=(DomainShaderReader&&) = default;
		~DomainShaderReader() = default;

	protected:
		virtual std::shared_ptr<DomainShader> _Read(const std::wstring& assetName) override;
	};
}