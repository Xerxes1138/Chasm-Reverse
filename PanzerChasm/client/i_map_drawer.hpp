#pragma once
#include <array>

#include <matrix.hpp>
#include <plane.hpp>

#include "map_state.hpp"
#include "weapon_state.hpp"

namespace PanzerChasm
{

// 0 - front
// 1, 2 - left, right
// 3, 4 - bottom, top
typedef std::array<m_Plane3, 5u> ViewClipPlanes;

class IMapDrawer
{
public:
	virtual ~IMapDrawer(){}

	virtual void SetMap( const MapDataConstPtr& map_data )= 0;

	virtual void Draw(
		const MapState& map_state,
		const m_Mat4& view_rotation_and_projection_matrix,
		const m_Vec3& camera_position,
		const ViewClipPlanes& view_clip_planes,
		EntityId player_monster_id )= 0;

	virtual void DrawWeapon(
		const WeaponState& weapon_state,
		const m_Mat4& projection_matrix,
		const m_Vec3& camera_position,
		float x_angle, float z_angle )= 0;
};

} // namespace PanzerChasm
