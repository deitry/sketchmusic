#pragma once

#include "../SketchMusic.h"

namespace SketchMusic
{
	public enum class ScaleCategory
	{
		Major,
		Minor,
		//HarmonicMinor,
		Custom
	};
}

/**
 * Символ, обозначающий переход на другую гамму.
 *
 * Пригодится при конкретизации относительных и обобщённых нот.
 */
public ref class SketchMusic::SScale sealed : public SketchMusic::ISymbol
{
public:
	SScale() {}
	SScale(NoteType type, ScaleCategory category)
	{
		_baseNote = type;
		_steps = ref new Platform::Collections::Map < Degree, DegreeInfo >;

		switch (category)
		{
			// TODO: как правильно заполнять "кастомные гаммы"?
			// может, вообще не стоит пользоваться "категориями"?
			case ScaleCategory::Minor:
			{
				_steps->Insert(Degree::I, DegreeInfo::Tone);
				_steps->Insert(Degree::II, DegreeInfo::Tone);
				_steps->Insert(Degree::III, DegreeInfo::Dim);
				_steps->Insert(Degree::IV, DegreeInfo::Tone);
				_steps->Insert(Degree::V, DegreeInfo::Tone);
				_steps->Insert(Degree::VI, DegreeInfo::Dim);
				_steps->Insert(Degree::VII, DegreeInfo::Tone);
			}
			default:
			{
				_steps->Insert(Degree::I, DegreeInfo::Tone);
				_steps->Insert(Degree::II, DegreeInfo::Tone);
				_steps->Insert(Degree::III, DegreeInfo::Tone);
				_steps->Insert(Degree::IV, DegreeInfo::Tone);
				_steps->Insert(Degree::V, DegreeInfo::Tone);
				_steps->Insert(Degree::VI, DegreeInfo::Tone);
				_steps->Insert(Degree::VII, DegreeInfo::Tone);
			}
		}
	}

	[Windows::Foundation::Metadata::DefaultOverloadAttribute]
	SScale(NoteType type, Windows::Foundation::Collections::IMap<Degree, DegreeInfo >^ steps)
	{
		_baseNote = type;
		_steps = steps;
	}

	property NoteType _baseNote;
	property Windows::Foundation::Collections::IMap<Degree, DegreeInfo >^ _steps;

	virtual SymbolType GetSymType() { return SymbolType::SCALE; }
	virtual Platform::String^ ToString() { return noteTypeToString(_baseNote); }

	// Унаследовано через ISymbol
	virtual bool EQ(ISymbol ^ second) { return false; }

	int ToValue()
	{
		int value = static_cast<int>(_baseNote);
		return (value <= 2) ? value : value - 12; // чтобы гамма в целом была ниже
	}
};