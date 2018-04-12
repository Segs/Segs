#pragma once
class Entity;
class BitStream;
struct ClientEntityStateBelief;
void serializeto(const Entity & src, ClientEntityStateBelief &belief, BitStream &bs);
void sendBuffs(const Entity &src,BitStream &bs);
