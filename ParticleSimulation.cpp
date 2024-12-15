#include <math.h>

#include "Math/WorldMath.h"

struct ColorRGB
{
  float r, g, b;
};


#define SIMULATION_PARTICLE_COUNT 512

struct Particle
{
  WorldVector WorldPosition = {0, 0, 0, 0};
  WorldVector Velocity = {0, 0};
  ColorRGB Color = {1.f, 1.f, 1.f};
  float Radius = 1.f;
  float Mass = 1;
  bool IsActive = false;
};

enum class SimulationInputKey : int
  {
    // Alphabet
    A, B, C, D, E, F, G, H, I, J, K, L, M, N,
    O, P, Q, R, S, T, U, V, W, X, Y, Z,

    // Numbers
    NUM_1, NUM_2, NUM_3, NUM_4, NUM_5, NUM_6, NUM_7, NUM_8, NUM_9, NUM_0,

    // Special Characters
    ESCAPE,
      
    KEY_COUNT
  };

enum class SimulationInputState
  {
    NONE,
    PRESSED,
    HELD,
    RELEASED
  };

// Contains all Simulation persistent data and platform layer functions.
struct SimulationContext
{
  SimulationInputState InputStates[static_cast<int>(SimulationInputKey::KEY_COUNT)];
  
  Particle Particles[SIMULATION_PARTICLE_COUNT];
  int TickCount = 0;
  
  void (*SendParticleRenderCommand)(Matrix4x4 TranformMatrix, ColorRGB Color, float Radius);
  void (*SetViewportMatrix)(Matrix4x4 ViewportMatrix);
  void (*SendExitApplicationCommand)();
  
  bool IsKeyPressed(SimulationInputKey Key) const
  {
    return InputStates[static_cast<int>(Key)] == SimulationInputState::PRESSED
      || InputStates[static_cast<int>(Key)] == SimulationInputState::HELD;
  }
};

Particle CreateParticle(ColorRGB Color, float Radius, WorldVector Position)
{
  Particle NewParticle;
  NewParticle.IsActive = true;
  NewParticle.Color = Color;
  NewParticle.Radius = Radius;
  NewParticle.WorldPosition = Position;
  NewParticle.Velocity = {0, 0, 0, 0};
  NewParticle.Mass = 1;
  
  return NewParticle;
}

// Returns new World Position of all Particles.
void ProcessParticlePhysics(SimulationContext& Context, float TimeDelta, WorldVector* OutNewPositions)
{ 
  WorldVector NewPositions[SIMULATION_PARTICLE_COUNT];
  memset(NewPositions, 0, sizeof(NewPositions));
    
  for (int ParticleIndex = 0; ParticleIndex < SIMULATION_PARTICLE_COUNT; ParticleIndex++)
    {
      if (!Context.Particles[ParticleIndex].IsActive)
	{
	  continue;
	}

      WorldVector Velocity = Context.Particles[ParticleIndex].Velocity;
    
      WorldVector TotalGravForce;
      for(int OtherParticleIndex = 0; OtherParticleIndex < SIMULATION_PARTICLE_COUNT; OtherParticleIndex++)
	{
	  if (!Context.Particles[OtherParticleIndex].IsActive || ParticleIndex == OtherParticleIndex)
	    {
	      continue;
	    }

	  WorldVector ToOther = Context.Particles[OtherParticleIndex].WorldPosition - Context.Particles[ParticleIndex].WorldPosition;
	  float dSquared = LengthSquared(ToOther) * 1000;
	  if (dSquared > 1)
	    {
	      WorldVector GravForce = NormalizeVector(ToOther) * (Context.Particles[ParticleIndex].Mass * Context.Particles[OtherParticleIndex].Mass / dSquared);

	      TotalGravForce = TotalGravForce + GravForce;
	    }
	}

      Velocity = Velocity + (TotalGravForce / Context.Particles[ParticleIndex].Mass * TimeDelta);
      
      WorldVector NewPosition = Context.Particles[ParticleIndex].WorldPosition + Velocity * TimeDelta; 
      OutNewPositions[ParticleIndex] = NewPosition;
      Context.Particles[ParticleIndex].Velocity = Velocity;
    }
}

void RunSimulation(SimulationContext& Context, float TimeDelta)
{
  if (Context.TickCount == 0)
    {
      Context.Particles[0] = CreateParticle({1, 0, 1}, 0.05f, {0, 0, 0, 1});
      Context.Particles[0].Mass = 500;

      for(int ParticleIndex = 1; ParticleIndex < SIMULATION_PARTICLE_COUNT; ParticleIndex++)
	{
	  ColorRGB PartCol;
	  WorldVector PartPos;
	  WorldVector PartVel;
	  
	  PartCol = {1, 0, 0};
	  PartPos = {0, (rand() % 100 - 50) / 100.f, (rand() % 100 - 50) / 100.f};
	  PartVel = {0, (rand() % 100 - 50) / 100.f * 1, (rand() % 100 - 50) / 100.f * 1};
	  
	  Context.Particles[ParticleIndex] = CreateParticle(PartCol, 0.01f, PartPos);
	  Context.Particles[ParticleIndex].Velocity = PartVel;
	}

      Context.TickCount = 1;
      return;
    }
  
  WorldVector NewPositions[SIMULATION_PARTICLE_COUNT];
  ProcessParticlePhysics(Context, TimeDelta, NewPositions);

  // Update particle positions after physics tick.

  for(int ParticleIndex = 0; ParticleIndex < SIMULATION_PARTICLE_COUNT; ParticleIndex++)
    {
      Context.Particles[ParticleIndex].WorldPosition = NewPositions[ParticleIndex];
    }
  
  // Draw all active Particles.
  for (int ParticleIndex = 0; ParticleIndex < SIMULATION_PARTICLE_COUNT; ParticleIndex++)
    {
      Particle& Part = Context.Particles[ParticleIndex];
      if (!Part.IsActive)
	{
	  continue;
	}

      Matrix4x4 ParticleMatrix = Matrix4x4::Identity;
      ParticleMatrix.SetTranslation(Part.WorldPosition);
      ParticleMatrix.SetScale(Part.Radius, Part.Radius, Part.Radius);
      
      Context.SendParticleRenderCommand(ParticleMatrix, Part.Color, Part.Radius); 
    }

  static Matrix4x4 CameraTransform = Matrix4x4::Identity;

  WorldVector CameraMovementVector = WorldVector::ZeroVector;
  
  if (Context.IsKeyPressed(SimulationInputKey::Z))
    {
      CameraMovementVector = CameraMovementVector + WorldVector::UpVector;
    }
  if (Context.IsKeyPressed(SimulationInputKey::Q))
    {
      CameraMovementVector = CameraMovementVector - WorldVector::RightVector;
    }
  if (Context.IsKeyPressed(SimulationInputKey::S))
    {
      CameraMovementVector = CameraMovementVector - WorldVector::UpVector;
    }
  if (Context.IsKeyPressed(SimulationInputKey::D))
    {
      CameraMovementVector = CameraMovementVector + WorldVector::RightVector;
    }
  if (Context.InputStates[static_cast<int>(SimulationInputKey::ESCAPE)] == SimulationInputState::RELEASED)
    {
      Context.SendExitApplicationCommand();
    }
  
  CameraTransform.AddTranslation(CameraMovementVector * TimeDelta * 2);
  Context.SetViewportMatrix(CameraTransform);
  
  Context.TickCount++;
}
