#pragma once
class Entity;
class BitStream;
class MapClient;
class ColorAndPartPacker;
struct ClientEntityStateBelief;
void serializeto(const Entity & src, ClientEntityStateBelief &belief, BitStream &bs);
void sendBuffs(const Entity &src,BitStream &bs);
