#pragma once
class Entity;
class BitStream;
class MapClient;
class ColorAndPartPacker;
void serializeto(const Entity & src, BitStream &bs);
void sendBuffs(const Entity &src,BitStream &bs);
