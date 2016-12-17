#include "pch.h"
#include "SoundFont.h"
#include "../SketchMusic.h"

using namespace concurrency;
using namespace Windows::Storage;
using namespace Windows::Storage::Streams;
using namespace Platform;
using namespace SketchMusic::SFReader;


/**
Пусть лучше впредь возвращает xaudio2 буфер
*/
void SketchMusic::SFReader::SFPreset::noteOn(int key, int vel)
{
	SFPresetZone^ gPresZone = this->globalZone;
	
	// run thru all the zones of this preset
	for (auto pZone : this->zones)
	{
		// check if the note falls into the key and velocity range of this preset
		//if (pZone->inZone(key, vel)) 
		//{

			SFInstrument^ inst = pZone->instrument;
			SFInstrumentZone^ gInstZone = inst->globalZone;
			
			// run thru all the zones of this instrument
			for (auto iZone : inst->zones)
			{
				// make sure this instrument zone has a valid sample
				SFSample^ sample = iZone->sample;
				if (sample == nullptr) continue;

				// check if the note falls into the key and velocity range of this
				// instrument
				//if (iZone->inZone(key,vel))
				//{
					// !!!
					// мы тут будем получать голоса по-другому. Нечего их инициализировать,
					// когда мы собираемся ноту играть. Их нам каким-нибудь образом будет предоставлять
					// SoundEngine. А ещё лучше, если сама эта функция будет методом SoundEngine
					// Требуется по одному голосу на каждый инструмент? Ну да, у нас же каждому "инструменту" соответствует сэмпл
					
					//SFVoice^ voice;

					// Instrument level, generators

					//for (i = 0; i < GEN_LAST; i++) {
					/*for (auto gen : iZone->generators)
					{
						// SF 2.01 section 9.4 'bullet' 4:
						// A generator in a local instrument zone supersedes a
						// global instrument zone generator.  Both cases supersede
						// the default generator -> voice_gen_set

						if (gen->flags)
						{
							voice->setGenerator(gen);
						}
						else if ((gInstZone != nullptr) && (gen->flags))
						{
							voice->setGenerator(gInstZone->generators->GetAt(gen->i));
						}
						else {
							// The generator has not been defined in this instrument.
							// Do nothing, leave it at the default.
						}

					} // for all iZone generators
					*/
					// global instrument zone, modulators: Put them all into a
					// list.
					if (gInstZone != nullptr)
					{
						for (auto mod : gInstZone->modulators)
						{
							//voice->addModulator(mod);
						}
					}

					// local instrument zone, modulators.
					// Replace modulators with the same definition in the list:
					// SF 2.01 page 69, 'bullet' 8
					for (auto mod : iZone->modulators)
					{
						//voice->addModulator(mod);
					}

					// Preset level, generators
					/*for (int i = 0; i < GEN_LAST; i++)
					{
						auto gen = pZone->generators->GetAt(i);

						if (SketchMusic::SFGenerator::ignoreAtPresetLevel(gen))
						{
							continue;
						}

						// SF 2.01 section 9.4 'bullet' 9: A generator in a
						// local preset zone supersedes a global preset zone
						// generator.  The effect is -added- to the destination
						// summing node -> voice_gen_incr

						// ТУТ НАДО ПОРАБОТАТЬ

						if (gen->flags) {
							voice->genIncr(gen);
						}
						else if (gPresZone != nullptr)
						{
							auto gen2 = gPresZone->generators->GetAt(i);
							if (gen2->flags) {
								voice->genIncr(gen2);
							}
						}
						else {
							// The generator has not been defined in this preset
							// Do nothing, leave it unchanged.
						}
						i++;
					} // for all pZone generators
					*/

					// Global preset zone, modulators: put them all into a list.
					if (gPresZone != nullptr)
					{
						for (auto mod : gPresZone->modulators)
						{
							//voice->addModulator(mod);
						}
					}

					// Process the modulators of the local preset zone.  Kick
					// out all identical modulators from the global preset zone
					// (SF 2.01 page 69, second-last bullet)
					for (auto mod : pZone->modulators)
					{
						//voice->addModulator(mod);
					}

					// add the synthesis process to the synthesis loop.
					//voice->start();
				}
			//}
		}
	//}
}

// непосредственно загрузка SFData
#pragma region sfload

/*SketchMusic::SFLoader::SFLoader(Platform::String^ fileName)
{
	stream = ref new FileStream(fileName);
	reader = ref new DataReader(stream);
	
	// настройка потока
	reader->UnicodeEncoding = UnicodeEncoding::Utf8;
	reader->ByteOrder = ByteOrder::LittleEndian;
}*/

/**
Проходимся по всем зонам и заполняем их валидными значениями
*/
void SketchMusic::SFReader::SFPreset::updateParams()
{
	for (auto pZone : this->zones)
	{
		if (pZone->instrument)
		{
			for (auto iZone : pZone->instrument->zones)
			{
				for (auto gen : iZone->generators)
				{
					switch (gen.first)
					{
					case SFGeneratorID::keyRange:
						iZone->keyHi = gen.second->val.range.hi;
						iZone->keyLo = gen.second->val.range.lo;
						break;
					case SFGeneratorID::velRange:
						iZone->velHi = gen.second->val.range.hi;
						iZone->velLo = gen.second->val.range.lo;
						break;
					default:
						break;
					}
				}
			}
		}
		for (auto gen : pZone->generators)
		{
			switch (gen.first)
			{
			case SFGeneratorID::keyRange:
				pZone->keyHi = gen.second->val.range.hi;
				pZone->keyLo = gen.second->val.range.lo;
				break;
			case SFGeneratorID::velRange:
				pZone->velHi = gen.second->val.range.hi;
				pZone->velLo = gen.second->val.range.lo;
				break;
			default:
				break;
			}
		}
	}
}