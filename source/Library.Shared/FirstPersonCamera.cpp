#include "pch.h"
#include "FirstPersonCamera.h"
#include "MouseComponent.h"
#include "KeyboardComponent.h"
#include "Game.h"
#include "VectorHelper.h"

using namespace std;
using namespace DirectX;

namespace Library
{
    RTTI_DEFINITIONS(FirstPersonCamera)

    FirstPersonCamera::FirstPersonCamera(Game& game, float fieldOfView, float aspectRatio, float nearPlaneDistance, float farPlaneDistance) :
		PerspectiveCamera(game, fieldOfView, aspectRatio, nearPlaneDistance, farPlaneDistance)
    {
    }

	GamePadComponent* FirstPersonCamera::GetGamePad() const
	{
		return mGamePad;
	}

	void FirstPersonCamera::SetGamePad(GamePadComponent* gamePad)
	{
		mGamePad = gamePad;
	}

	KeyboardComponent* FirstPersonCamera::GetKeyboard() const
	{
		return mKeyboard;
	}

	void FirstPersonCamera::SetKeyboard(KeyboardComponent* keyboard)
	{
		mKeyboard = keyboard;
	}

	MouseComponent* FirstPersonCamera::GetMouse() const
	{
		return mMouse;
	}

	void FirstPersonCamera::SetMouse(MouseComponent* mouse)
	{
		mMouse = mouse;
	}

	float& FirstPersonCamera::MouseSensitivity()
	{
		return mMouseSensitivity;
	}

    float& FirstPersonCamera::RotationRate()
    {
        return mRotationRate;
    }

    float& FirstPersonCamera::MovementRate()
    {
        return mMovementRate;
    }

	function<void()> FirstPersonCamera::PositionUpdatedCallback() const
	{
		return mPositionUpdatedCallback;
	}

	void FirstPersonCamera::SetPositionUpdatedCallback(function<void()> callback)
	{
		mPositionUpdatedCallback = callback;
	}

	void FirstPersonCamera::SetPosition(float x, float y, float z)
	{
		Camera::SetPosition(x, y, z);
		if (mPositionUpdatedCallback != nullptr)
		{
			mPositionUpdatedCallback();
		}
	}

	void FirstPersonCamera::SetPosition(FXMVECTOR position)
	{
		Camera::SetPosition(position);
		if (mPositionUpdatedCallback != nullptr)
		{
			mPositionUpdatedCallback();
		}
	}

	void FirstPersonCamera::SetPosition(const XMFLOAT3& position)
	{
		Camera::SetPosition(position);
		if (mPositionUpdatedCallback != nullptr)
		{
			mPositionUpdatedCallback();
		}		
	}

	void FirstPersonCamera::Initialize()
	{
		mGamePad = reinterpret_cast<GamePadComponent*>(mGame->Services().GetService(GamePadComponent::TypeIdClass()));
		mKeyboard = (KeyboardComponent*)mGame->Services().GetService(KeyboardComponent::TypeIdClass());
		mMouse = (MouseComponent*)mGame->Services().GetService(MouseComponent::TypeIdClass());

		Camera::Initialize();
	}

    void FirstPersonCamera::Update(const GameTime& gameTime)
    {		
		GamePad::State gamePadState;
		if (IsGamePadConnected(gamePadState))
		{
			XMFLOAT2 movementAmount(gamePadState.thumbSticks.leftX, gamePadState.thumbSticks.leftY);
			XMFLOAT2 rotationAmount(-gamePadState.thumbSticks.rightX, gamePadState.thumbSticks.rightY);

			if (movementAmount.x != 0 || movementAmount.y != 0 || rotationAmount.x != 0 || rotationAmount.y != 0)
			{
				UpdatePosition(movementAmount, rotationAmount, gameTime);
			}
		}
		else
		{
			bool positionChanged = false;
			XMFLOAT2 movementAmount = Vector2Helper::Zero;
			if (mKeyboard != nullptr)
			{				
				if (mKeyboard->IsKeyDown(Keys::W))
				{
					movementAmount.y = 1.0f;
					positionChanged = true;
				}
				if (mKeyboard->IsKeyDown(Keys::S))
				{
					movementAmount.y = -1.0f;
					positionChanged = true;
				}
				if (mKeyboard->IsKeyDown(Keys::A))
				{
					movementAmount.x = -1.0f;
					positionChanged = true;
				}
				if (mKeyboard->IsKeyDown(Keys::D))
				{
					movementAmount.x = 1.0f;
					positionChanged = true;
				}
			}

			XMFLOAT2 rotationAmount = Vector2Helper::Zero;
			if (mMouse != nullptr && mMouse->Mode() == MouseModes::Relative)
			{				
				if (mMouse->IsButtonHeldDown(MouseButtons::Left))
				{
					rotationAmount.x = -mMouse->X() * mMouseSensitivity;
					rotationAmount.y = -mMouse->Y() * mMouseSensitivity;
					positionChanged = true;
				}
			}

			if (positionChanged)
			{
				UpdatePosition(movementAmount, rotationAmount, gameTime);
			}
		}

        Camera::Update(gameTime);
    }

	void FirstPersonCamera::UpdatePosition(const XMFLOAT2& movementAmount, const XMFLOAT2& rotationAmount, const GameTime& gameTime)
	{
		float elapsedTime = gameTime.ElapsedGameTimeSeconds().count();
		XMVECTOR rotationVector = XMLoadFloat2(&rotationAmount) * mRotationRate * elapsedTime;
		XMVECTOR right = XMLoadFloat3(&mRight);

		XMMATRIX pitchMatrix = XMMatrixRotationAxis(right, XMVectorGetY(rotationVector));
		XMMATRIX yawMatrix = XMMatrixRotationY(XMVectorGetX(rotationVector));

		ApplyRotation(XMMatrixMultiply(pitchMatrix, yawMatrix));

		XMVECTOR position = XMLoadFloat3(&mPosition);
		XMVECTOR movement = XMLoadFloat2(&movementAmount) * mMovementRate * elapsedTime;

		XMVECTOR strafe = right * XMVectorGetX(movement);
		position += strafe;

		XMVECTOR forward = XMLoadFloat3(&mDirection) * XMVectorGetY(movement);
		position += forward;

		XMStoreFloat3(&mPosition, position);

		if (mPositionUpdatedCallback != nullptr)
		{
			mPositionUpdatedCallback();
		}
	}
}