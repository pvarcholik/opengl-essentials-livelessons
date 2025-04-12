#pragma once

#include "RTTI.h"
#include <gsl/gsl>

namespace Library
{
	class Game;
	class GameTime;

	class GameComponent : public RTTI
	{
		RTTI_DECLARATIONS(GameComponent, RTTI)

	public:
		GameComponent() = default;
		explicit GameComponent(Game& game);
		GameComponent(const GameComponent&) = default;
		GameComponent& operator=(const GameComponent&) = default;
		GameComponent(GameComponent&&) = default;
		GameComponent& operator=(GameComponent&&) = default;
		virtual ~GameComponent() = default;

		gsl::not_null<Game*> GetGame() const;
		void SetGame(Game& game);
		bool Enabled() const;
		void SetEnabled(bool enabled);

		virtual void Initialize();
		virtual void Update(const GameTime& gameTime);

	protected:
		gsl::not_null<Game*> mGame;
		bool mEnabled =  true;
	};
}

#include "GameComponent.inl"